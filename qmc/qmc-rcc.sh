#!/bin/sh

if [ $# -lt 3 ]; then
    echo "Usage: $(basename $0) qmc-options output-file input-qrc-files [rcc-options...]"
    echo "  qmc-options     : qmc generic options as a single string or empty."
    echo "  output-file     : Output file name."
    echo "  input-qrc-files : Input qrc files to use, at least one."
    echo "  rcc-options     : Options that are passed to rcc."
    exit 1
fi

# Helper to keep track of directories and files to delete. Avoids duplicates.
function contains() {
    S=$1
    SUB=$2
    if test "${S#*$SUB}" != "$S"
    then
        return 0
    fi
    return 1
}

# Process command-line parameters.
QMCFLAGS=$1
shift
OUT=$1
DEST=$(dirname $OUT)
shift
INS=
while [ $# -gt 0 ]
do
    # Check if it's a qrc file.
    F=$(basename $1)
    E="${F##*.}"
    if [ "x$E" != "xqrc" ]; then
        break
    fi
    INS=$(echo $INS" "$1)
    shift
done
RCCFLAGS=$*

# Process all input .qrc files so that file names are modified and intermediate
# files are kept track of.
RMDIRS=
RMFILES=
CINS=
TMP=$(echo $DEST/tmp$$)
for F in $INS
do
    B=$(basename $F)
    D=$(dirname $F)
    TGT=$(echo $DEST/_$B)
    CINS=$(echo $CINS" "$TGT)
    cat $F | sed -e 's/\.qml</.qmc</g' -e 's/\.js</.jsc</g' > "$TGT"
    # Find all files and compile or just copy.
    cat $F | awk -F '</file>' '{ if (NF > 1) for (k = 1; k < NF; k++) { c = split($k, a, ">"); print a[c]; } }' >$TMP
    for L in $(cat $TMP)
    do
        T=$(echo $L | sed -e 's/\.qml$/.qmc/g' -e 's/\.js$/.jsc/g')
        TF=$(echo $DEST/$T)
        TD=$(dirname $TF)
        SF=$(echo $D/$L)
        # Keep track of files and directories to be deleted afterwards.
        if [ "$DEST" != "$D" ]; then
            # Shadow build. Later delete directories and files we create.
            if [ "x$TD" == "x$DEST" ]; then
                RMFILES=$(echo " "$RMFILES" "$TF" ")
            else
                # Assumes that there will be nothing else in these directories.
                contains "$RMDIRS" " $TD " || RMDIRS=$(echo " "$RMDIRS" "$TD" ")
            fi
            mkdir -p $TD
            if [ $T == $L ]; then
                cp $SF $TF
            else
                qmc $QMCFLAGS $SF -o $TF
            fi
        else
            # Not a shadow build. Delete only files we generate.
            if [ $T != $L ]; then
                qmc $QMCFLAGS $SF -o $TF
                RMFILES=$(echo " "$RMFILES" "$TF" ")
            fi
        fi
    done
    rm -f $TMP
done

cd $DEST
rcc $RCCFLAGS -name res $CINS -o $(basename $OUT)
# Cleaning could be done for full file list and only empty directories removed.
# Technically, could leave files to be and check if there is need to compile or
# copy as that would save time during build. Later.
rm -rf $RMDIRS
rm -f $RMFILES $CINS


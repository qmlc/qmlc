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
FAIL=0
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
            elif [ ! -d "$TD" ]; then
                # Make the directory and mark it for deletion later.
                mkdir -p $TD
                if [ $? -ne 0 ]; then
                    # There was a file by that name?
                    echo "Error: failed to make directory $TD"
                    FAIL=1
                    break
                fi
                RMDIRS=$(echo " "$RMDIRS" "$TD" ")
            else
                # If the directory is not to be removed, remove the file only.
                contains "$RMDIRS" " $TD " || RMFILES=$(echo " "$RMFILES" "$TF" ")
            fi
            if [ $T == $L ]; then
                cp $SF $TF
            else
                echo "qmc $SF"
                qmc $QMCFLAGS $SF -o $TF -n $L
                if [ $? -ne 0 ]; then
                    FAIL=2
                    break
                fi
            fi
        else
            # Not a shadow build. Delete only files we generate.
            if [ $T != $L ]; then
                echo "qmc $SF"
                qmc $QMCFLAGS $SF -o $TF -n $L
                if [ $? -ne 0 ]; then
                    FAIL=2
                    break
                fi
                RMFILES=$(echo " "$RMFILES" "$TF" ")
            fi
        fi
    done
    rm -f $TMP
done

if [ $FAIL -eq 0 ]; then
    ORIG=$(pwd)
    cd $DEST
    rcc $RCCFLAGS -name res $CINS -o $(basename $OUT)
    FAIL=$?
    cd $ORIG
fi
rm -rf $RMDIRS
rm -f $RMFILES $CINS
exit $FAIL


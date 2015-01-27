function callback(result)
{
    console.log("Qt.include from includer3.");
}

Qt.include("includerd.js", callback);

function test()
{
    return otherFunc("includer3");
}


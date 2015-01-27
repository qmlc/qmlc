function callback(result)
{
    console.log("Qt.include from includer.");
}

Qt.include("included.js", callback);

function test()
{
    return someFunc("includer");
}


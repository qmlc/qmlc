function callback(result)
{
    console.log("Qt.include from includer2.");
}

function test()
{
    Qt.include("included.js", callback);
    return someFunc("includer2");
}


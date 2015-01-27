function callback(result)
{
    console.log("Qt.include from includer2d.");
}

Qt.include("included.js", callback);

function thirdFunc(item)
{
    return someFunc(item);
}


function callback(result)
{
    console.log("Qt.include from includerd.");
}

Qt.include("includer2d.js", callback);

function otherFunc(item)
{
    return thirdFunc(item);
}


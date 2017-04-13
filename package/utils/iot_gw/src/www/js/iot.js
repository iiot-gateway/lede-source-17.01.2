if (typeof console == "undefined") {
    this.console = {log: function() {}};
}

function xml2Str(xmlNode)
{
  try {
    // Gecko-based browsers, Safari, Opera.
    return (new XMLSerializer()).serializeToString(xmlNode);
  }
  catch (e) {
    try {
      // Internet Explorer.
      return xmlNode.xml;
    }
    catch (e)
    {//Strange Browser ??
     alert('Xmlserializer not supported');
    }
  }
  return false;
}
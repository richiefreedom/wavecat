function AJAXCreate()
{
	var xmlhttp;	

	if (window.XMLHttpRequest)
		xmlhttp = new XMLHttpRequest();
	else
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");

	return xmlhttp;
}

function AJAXSendTextPost(xmlhttp, url, text, callback)
{
	xmlhttp.open("POST", url, true);
	xmlhttp.setRequestHeader("Content-type",
		"application/x-www-form-urlencoded");
	xmlhttp.onreadystatechange = callback;
	xmlhttp.send(text);
}

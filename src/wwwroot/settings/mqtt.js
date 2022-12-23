function saveMQTT() {
    const body = getUpdateBody();

    executeCall("/settings/mqtt", "PUT", body).then((updOk) => {
        if (updOk == true) {
            document.location.reload(true);
        }
    });
}

function getUpdateBody() {
    const server = document.getElementById("server").value;
    const user = document.getElementById("user").value;
    const pw = document.getElementById("pw").value;
    const port = document.getElementById("port").value;
    const cert = document.getElementById("certificate").value;
    const arrCert = cert.split("\n");

    var body = '{"server":"' + server + '","user":"' + user + '","pw":"' + pw + '","port":' + port + ',"cert":[';
    var added = 0;
    for (var i = 0; i < arrCert.length; i++) {
        if (arrCert[i] != "") {
            if (added > 0) {
                body += ',';
            }
            added++;
            body += '"' + arrCert[i].replace("\r", "") + '"';
        }
    }
    body += ']}';
    return body;
}

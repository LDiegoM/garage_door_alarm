function saveDate() {
    const body = getUpdateBody();

    executeCall("/settings/date", "PUT", body).then((updOk) => {
        if (updOk == true) {
            document.location.reload(true);
        }
    });
}

function getUpdateBody() {
    const server = document.getElementById("server").value;
    const gmtOffset = document.getElementById("gmtOffset").value;
    const daylightOffset = document.getElementById("daylightOffset").value;

    var body = '{"server":"' + server + '","gmt_offset":' + gmtOffset + ',"daylight_offset":' + daylightOffset + '}';
    return body;
}

function executeDeleteAP(ap) {
    var ok = true;
    if (ap != "") {
        executeCall("/settings/wifi?ap=" + ap, "DELETE").then((callOK) => {
            ok = callOK;
        });
    }
    if (ok) {
        document.location.reload(true);
    }
}

async function executeUpdateAPs() {
    const aps = document.getElementsByClassName('upd-ap');
    const arrAPs = [...aps].map(input => input.value);
    const pws = document.getElementsByClassName('upd-pw');
    const arrPws = [...pws].map(input => input.value);
    const body = getUpdateBody(arrAPs, arrPws);

    var ok = false;
    if (body != '{"aps":[]}') {
        ok = await executeCall("/settings/wifi", "PUT", body);
    }
    return ok;
}

function getUpdateBody(arrAPs, arrPws) {
    var s = '{"aps":[';
    var added = 0;
    for (var i = 0; i < arrAPs.length; i++) {
        if (arrPws[i] != "****") {
            if (added > 0) {
                s += ",";
            }
            added++;
            s += '{"ap":"' + arrAPs[i] + '", "pw":"' + arrPws[i] + '"}';
        }
    }
    s += "]}";
    return s;
}

async function executeInsertAP() {
    const ap = document.getElementById("ap").value;
    const pw = document.getElementById("pw").value;
    var ok = false;
    if (ap != "") {
        const body = '{"ap":"' + ap + '","pw":"' + pw + '"}';
        ok = await executeCall("/settings/wifi", "POST", body);
    }
    return ok;
}

function saveWiFi() {
    var okUpd = false;
    var okAdd = false;
    executeUpdateAPs().then((updOk) => {
        okUpd = updOk;
        executeInsertAP().then((addOk) => {
            okAdd = addOk;

            if (okUpd == true || okAdd == true) {
                document.location.reload(true);
            }
        });
    });
}

function restart() {
    executeCall("/restart", "POST").then((updOk) => {
        if (updOk == true) {
            openModal("Done", "garage_door_alarm is restarting");
        }
    });
}
function clearSettings() {
    executeCall("/settings", "DELETE").then((updOk) => {
        if (updOk == true) {
            openModal("Done", "Successfully removed current settings");
        }
    });
}
function downloadLogs() {
    window.location.assign("/logs");
}
function clearLogs() {
    executeCall("/logs", "DELETE").then((updOk) => {
        if (updOk == true) {
            document.location.reload(true);
        }
    });
}

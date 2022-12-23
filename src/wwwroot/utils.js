async function executeCall(path, method, body) {
    console.log("Executing command " + method + " to " + path);
    console.log("body: " + body);
    var ok = false;
    const response = await fetch(
        path,
        {
            method: method,
            headers: {
                'Content-Type': 'application/json'
            },
            body: body
        }
    )

    if (response.status >= 200 && response.status <= 299) {
        ok = true;
    } else if (response.status >= 400) {
        const resp = await response.text();
        openModal("configuration error", resp);
    }
    return ok;
}

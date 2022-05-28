const crudPOST = async () => {
    console.log("I am here");
    const response = await fetch('http://localhost:8080/api/Shoes/', {
        method: 'post',
        body: {"a": "haha"},
        headers: {
            'Content-Type': 'application/json'
        }
    });
    const msg = await response.text();
    console.log(msg);
}

const textPOST = async () => {
    document.getElementById("text_response").innerHTML = "";
    document.getElementById("wait").innerHTML = "Please wait for several seconds";
    var title = document.getElementById('title').value;
    var prompt = document.getElementById('prompt').value;
	var text_body = "title=" + title + "&prompt=" + prompt;
    const response = await fetch('http://localhost:8080/text_generate', {
        method: 'post',
        body: text_body,
    });
    document.getElementById("wait").innerHTML = "Here is the result:";
    document.getElementById("text_response").innerHTML = await response.text();
}

const crudGET = async () => {
    const response = await fetch('http://localhost:8080/api/Shoes/');
    const myJson = await response.json();
    console.log(myJson);
}
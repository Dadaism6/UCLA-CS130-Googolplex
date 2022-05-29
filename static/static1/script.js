const crudPOST = async () => {
    console.log("I am here");
    const response = await fetch('http://34.82.72.149:80/api/Shoes/', {
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
    document.getElementById("history").innerHTML = "";
    document.getElementById("wait").innerHTML = "Please wait for several seconds";
    var title = document.getElementById('title').value;
    var prompt = document.getElementById('prompt').value;
    var text_body = "title=" + title + "&prompt=" + prompt;
    const response = await fetch('http://34.82.72.149:80/text_generate', {
        method: 'POST',
        body: text_body,
    });
    document.getElementById("wait").innerHTML = "Here is the result:";
    document.getElementById("text_response").innerHTML = await response.text();
}

const textGET = async () => {
    const response = await fetch('http://34.82.72.149:80/text_generate/Hello');
    const myJson = await response.json();
}

const delete_text = async (target) => {
    const url = 'http://34.82.72.149:80/text_generate/' + target
    const response = await fetch(url, {
        method: 'DELETE',
    });
    await historyGET();
}

const get_single_history = async (target) => {
    const url = 'http://34.82.72.149:80/text_generate/' + target
    const response = await fetch(url);
    const history_parsed = await response.json();
    return history_parsed;
}

const historyGET = async () => {
    document.getElementById("text_response").innerHTML = "";
    const response = await fetch('http://34.82.72.149:80/text_generate/history?');
    const history = await response.json();
    let history_list = history['title list']
    history_list = history_list.substring(1, history_list.length - 1);
    history_list = history_list.split(',');

    const table_head = "<table>\n" +
        "  <tr onClick='viewHidden(this)'>\n" +
        "    <th>Index</th>\n" +
        "    <th>Title</th>\n" +
        "    <th>Prompt</th>\n" +
        "  </tr>"
    let tables = "";
    for (let i = 0; i < history_list.length; i++) {
        let curr = await get_single_history(history_list[i]);
        let content = curr['content'];
        tables += "<tr onClick='viewHidden(this)'>" +
            "<td>" + i + "</td>" +
            "<td>" + content['title'] + "</td>" +
            "<td>" + content['text_prompt'] + "</td>" +
            "<td class='hidden-row hidden-element'>" + content['output'] + "<input type=\"submit\" value=\"Delete\" onclick=\"delete_text('" + history_list[i] + "')\"><br><br>" + "</td>" +
            "</tr>";
    }
    document.getElementById("history").innerHTML = table_head + tables + '</table>';
}

const viewHidden = (element) => {
    element.getElementsByClassName('hidden-row')[0].classList.toggle('hidden-element');
}
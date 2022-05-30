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
    document.getElementById("wait").innerHTML = `<div class="spinner-grow text-primary" role="status">
                                                    <span class="sr-only"></span>
                                                </div>
                                                Loading...`;
    var title = document.getElementById('title').value;
    var prompt = document.getElementById('prompt').value;
    var text_body = "title=" + title + "&prompt=" + prompt;
    const response = await fetch('http://34.82.72.149:80/text_generate', {
        method: 'POST',
        body: text_body,
    });
    if (response.ok) {
        document.getElementById("wait").innerHTML = 'Here is the result:';
        document.getElementById("text_response").innerHTML = await response.text();
    } else {
        document.getElementById("wait").innerHTML = 'Error occurs:';
        document.getElementById("text_response").innerHTML = "<h4>Bad Request</h4>Whoops, errors happen in the process: <ul>\
            <li>storing the data to server\\'s filesystem</li></ul>Please wait for a moment and try again";
    }
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
    document.getElementById("wait").innerHTML = "";
    const response = await fetch('http://34.82.72.149:80/text_generate/history?');
    const table_head = "<table>\n" +
    "  <tr onClick='viewHidden(this)'>\n" +
    "    <th>Index</th>\n" +
    "    <th>Title</th>\n" +
    "    <th>Prompt</th>\n" +
    "  </tr>";

    if ( !response.ok) {
        document.getElementById("history").innerHTML = table_head + '</table>';
    } else {
        const history = await response.json();
        let history_list = history['title list'];

        history_list = history_list.substring(1, history_list.length - 1);
        if ( !history_list.length) {
            history_list = [];
        } else {
            history_list = history_list.split(',');
        }
        if (history_list.length === 0) {
            document.getElementById("history").innerHTML = "<div id=\"NoHistoryHelp\" class=\"black-form-text\"> No history to display, please try to submit some requests. </div>";
            return;
        }
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
}

const viewHidden = (element) => {
    element.getElementsByClassName('hidden-row')[0].classList.toggle('hidden-element');
}
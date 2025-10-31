const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><title>ESP32C3 Control</title></head>
<style>
  body {
    background: #6457A6;
    color: rgba(0, 0, 0, 0.4);
    font-family: sans-serif;
    text-align: center;
    margin-top: 50px;
  }

  input {
    width: 80%;
    margin-left: 3%;
    margin-top: 20%;
    border: none;
    border-radius: 6px;
    background: rgba(0, 0, 0, 0.2);
    color: #FFE347;
    font-size: 48px;
    font-family: sans-serif;
  }
  #sendButton {
    margin-top: 12px;
    background: rgba(255, 255, 255, 0.2);
    font-size: 48px;
  }
</style>
<body>

    <input type="text" id="textInput">
    <div>
      <button id="sendButton">send</button>
    </div>

    <script>

    document.getElementById('sendButton').addEventListener('click', () => {
    const input = document.getElementById('textInput').value
    console.log(input)

        fetch('/send-text', {
        method: 'POST',
        headers: {
          'Content-Type': 'text'
        },
        body: input
      })
      .then(response => {
        if (!response.ok) throw new Error('Network response not ok');
        return response.json();   // parse JSON
      })
      .then(data => {
        console.log('esp32 response:', data);
      })
      .catch(err => {
        console.error('post error:', err);
      })
    })

  </script>
</body>
</html>
)rawliteral";
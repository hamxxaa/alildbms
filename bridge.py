from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess
import threading
import queue
import os

app = Flask(__name__)
CORS(app)  # Allow frontend to access from different port

working_dir = os.path.dirname(os.path.abspath(__file__))

dbms_proc = subprocess.Popen(
    ["./dbms"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    text=True,
    bufsize=1,
    cwd=working_dir,  # This ensures DBMS can find ./databases
)

output_queue = queue.Queue()


def dbms_reader():
    buffer = []
    for line in dbms_proc.stdout:
        line = line.strip()
        if line == "!END!":
            output_queue.put(buffer)
            buffer = []
        else:
            buffer.append(line)


threading.Thread(target=dbms_reader, daemon=True).start()


@app.route("/query", methods=["POST"])
def query():
    sql = request.get_json().get("query", "").strip()
    if not sql:
        return jsonify({"error": "No query provided"}), 400

    dbms_proc.stdin.write(sql + "\n")
    dbms_proc.stdin.flush()

    try:
        result = output_queue.get(timeout=5)
        return jsonify(result)
    except queue.Empty:
        return jsonify({"error": "DBMS did not respond in time"}), 504


if __name__ == "__main__":
    app.run(port=5000)

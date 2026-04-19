const mqttStatus = document.getElementById("mqttStatus");
const logBox = document.getElementById("log");

const distanceEl = document.getElementById("distance");
const waterTextEl = document.getElementById("waterText");
const waterStatusEl = document.getElementById("waterStatus");

const dot1 = document.getElementById("dot1");
const dot2 = document.getElementById("dot2");
const dot3 = document.getElementById("dot3");
const dot4 = document.getElementById("dot4");

const switches = document.querySelectorAll('input[type="checkbox"]');

const historyBody = document.getElementById("historyBody");
const apiUrl = "https://69a2b493be843d692bd20691.mockapi.io/api/v1/water-monitor";

// ================= MQTT CONFIG =================
const options = {
    username: "uts-esp32-air",
    password: "uts-esp32-air"
};

const client = mqtt.connect(
    "wss://uts-esp32-air.cloud.shiftr.io:443",
    options
);

// ================= TOPIC =================
const topicWaterLevel = "uts/water/level";
const topicWaterStatus = "uts/water/status";
const topicLedStatus = "uts/led/status";
const topicLedControl = "uts/led/control";

// ================= CONNECT =================
client.on("connect", () => {
    mqttStatus.innerHTML = "● MQTT Terhubung";
    mqttStatus.style.background = "#16a34a";
    mqttStatus.style.color = "#ffffff";

    addLog("Berhasil terhubung ke Shiftr.io");

    client.subscribe(topicWaterLevel);
    client.subscribe(topicWaterStatus);
    client.subscribe(topicLedStatus);

    addLog("Subscribe: " + topicWaterLevel);
    addLog("Subscribe: " + topicWaterStatus);
    addLog("Subscribe: " + topicLedStatus);

    loadHistory();

    setInterval(() => {
        loadHistory();
    }, 5000);
});

// ================= MESSAGE =================
client.on("message", (topic, message) => {
    const data = message.toString();

    addLog("[" + topic + "] " + data);

    // ================= JARAK AIR =================
    if (topic === topicWaterLevel) {
        const value = parseFloat(data);

        if (!isNaN(value)) {
            distanceEl.innerText = value.toFixed(1);
        }
    }

    // ================= STATUS AIR =================
    if (topic === topicWaterStatus) {
        waterTextEl.innerText = data;
        waterStatusEl.innerText = data;

        if (data === "KOSONG") {
            waterStatusEl.style.background = "#dc2626";
        } else if (data === "RENDAH") {
            waterStatusEl.style.background = "#f97316";
        } else if (data === "SEDANG") {
            waterStatusEl.style.background = "#eab308";
        } else if (data === "TINGGI") {
            waterStatusEl.style.background = "#3b82f6";
        } else if (data === "PENUH") {
            waterStatusEl.style.background = "#16a34a";
        }

        waterStatusEl.style.color = "#ffffff";
    }

    // ================= STATUS LED =================
    if (topic === topicLedStatus) {
        dot1.classList.remove("active");
        dot2.classList.remove("active");
        dot3.classList.remove("active");
        dot4.classList.remove("active");

        // LED 1
        if (data.includes("L1:1")) {
            dot1.classList.add("active");
            switches[0].checked = true;
        } else {
            switches[0].checked = false;
        }

        // LED 2
        if (data.includes("L2:1")) {
            dot2.classList.add("active");
            switches[1].checked = true;
        } else {
            switches[1].checked = false;
        }

        // LED 3
        if (data.includes("L3:1")) {
            dot3.classList.add("active");
            switches[2].checked = true;
        } else {
            switches[2].checked = false;
        }

        // LED 4
        if (data.includes("L4:1")) {
            dot4.classList.add("active");
            switches[3].checked = true;
        } else {
            switches[3].checked = false;
        }
    }
});

// ================= ERROR =================
client.on("error", (err) => {
    mqttStatus.innerHTML = "● MQTT Gagal";
    mqttStatus.style.background = "#dc2626";
    mqttStatus.style.color = "#ffffff";

    addLog("ERROR: " + err.message);
});

// ================= DISCONNECT =================
client.on("close", () => {
    mqttStatus.innerHTML = "● MQTT Terputus";
    mqttStatus.style.background = "#ef4444";
    mqttStatus.style.color = "#ffffff";

    addLog("Koneksi MQTT terputus");
});

// ================= TOGGLE LED =================
function toggleLED(led, isOn) {
    const command = `LED${led}_${isOn ? "ON" : "OFF"}`;

    client.publish(topicLedControl, command);

    addLog("Kirim: " + command);
}

// ================= LOAD HISTORY =================
async function loadHistory() {
    try {
        historyBody.textContent = "Memuat data...";

        const response = await fetch(apiUrl);
        const data = await response.json();

        data.reverse();

        if (data.length === 0) {
            historyBody.textContent = "Belum ada data";
            return;
        }

        let output = "";

        data.slice(0, 10).forEach((item, index) => {
            const waktu = new Date(item.createdAt).toLocaleString("id-ID");

            output +=
`[${index + 1}]
Waktu : ${waktu}
Jarak : ${item.distance} cm
Status: ${item.status}
LED   : L1=${item.led1 ? "ON" : "OFF"}, L2=${item.led2 ? "ON" : "OFF"}, L3=${item.led3 ? "ON" : "OFF"}, L4=${item.led4 ? "ON" : "OFF"}

`;
        });

        historyBody.textContent = output;

    } catch (error) {
        historyBody.textContent = "Gagal mengambil data histori";
        addLog("ERROR histori: " + error.message);
    }
}

// ================= LOG =================
function addLog(text) {
    const time = new Date().toLocaleTimeString("id-ID");

    logBox.innerHTML += `[${time}] ${text}<br>`;
    logBox.scrollTop = logBox.scrollHeight;
}
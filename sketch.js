let serialPort; // 시리얼 포트를 저장할 변수
let writer; // 아두이노로 데이터를 전송하기 위한 writer 객체
let reader; // 아두이노에서 데이터를 읽기 위한 reader 객체
let redCycle = 1000; // 빨간불 주기
let yellowCycle = 500; // 노란불 주기
let greenCycle = 1500; // 초록불 주기

async function connectToArduino() {
    try {
        // 사용자가 연결할 시리얼 포트를 선택
        serialPort = await navigator.serial.requestPort();
        // 선택된 포트를 9600 baud로 열기
        await serialPort.open({ baudRate: 9600 });
        writer = serialPort.writable.getWriter();
        reader = serialPort.readable.getReader();
        console.log("아두이노 연결 성공");
        document.getElementById("connectionStatus").textContent = "현재 상태: 연결됨";

        // 데이터 읽기 시작
        readData();
    } catch (err) {
        console.error("연결 실패:", err);
        document.getElementById("connectionStatus").textContent = "현재 상태: 연결 실패";
    }
}

// 슬라이더 값 변경 시 호출되는 함수
function updateCycle(color) {
    let cycleValue;
    if (color === "red") {
        cycleValue = document.getElementById("redSlider").value;
        document.getElementById("redCycle").textContent = `${cycleValue} ms`;
        redCycle = cycleValue;
    } else if (color === "yellow") {
        cycleValue = document.getElementById("yellowSlider").value;
        document.getElementById("yellowCycle").textContent = `${cycleValue} ms`;
        yellowCycle = cycleValue;
    } else if (color === "green") {
        cycleValue = document.getElementById("greenSlider").value;
        document.getElementById("greenCycle").textContent = `${cycleValue} ms`;
        greenCycle = cycleValue;
    }

    // 새로운 값을 아두이노에 전송
    sendCycleData();
}

// 아두이노로 슬라이더 값(주기 데이터)을 전송
async function sendCycleData() {
    if (writer) {
        let data = `${redCycle},${yellowCycle},${greenCycle}\n`; // 데이터 포맷: "1000,500,1500\n"
        const dataArray = new TextEncoder().encode(data); // 데이터를 바이너리로 변환
        await writer.write(dataArray); // 데이터를 아두이노로 전송
        console.log("전송됨:", data);
    } else {
        console.log("아두이노에 연결되지 않았습니다.");
    }
}

// 아두이노로부터 데이터를 읽는 함수
async function readData() {
    const textDecoder = new TextDecoder();
    let dataBuffer = "";

    while (serialPort.readable) {
        const { value, done } = await reader.read();
        if (done) {
            console.log("연결이 닫혔습니다.");
            break;
        }
        if (value) {
            dataBuffer += textDecoder.decode(value);
            if (dataBuffer.includes("\n")) {
                const lines = dataBuffer.split("\n");
                while (lines.length > 1) {
                    const completeLine = lines.shift().trim();
                    processTrafficLightData(completeLine);
                }
                dataBuffer = lines.join("\n");
            }
        }
    }
}

// 신호등 상태 데이터를 처리하는 함수
function processTrafficLightData(data) {
    console.log("수신 데이터:", data);

    const redLight = document.getElementById("redLight");
    const yellowLight = document.getElementById("yellowLight");
    const greenLight = document.getElementById("greenLight");

    // 모든 신호등 초기화
    redLight.style.background = "gray";
    yellowLight.style.background = "gray";
    greenLight.style.background = "gray";

    // 아두이노에서 받은 데이터에 따라 신호등 색 변경
    if (data === "STATUS: RED LIGHT") {
        redLight.style.background = "red";
    } else if (data === "STATUS: YELLOW LIGHT") {
        yellowLight.style.background = "yellow";
    } else if (data === "STATUS: GREEN LIGHT") {
        greenLight.style.background = "green";
    }
}

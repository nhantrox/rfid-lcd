#ifndef WEBPAGE_H
#define WEBPAGE_H

// Giao diện web quản lý RFID - Trường ĐH Công Nghệ Thông Tin (UIT)
// Màu sắc chính thức UIT: Logo RGB(47, 107, 255) / Chữ RGB(0, 0, 253)

const char WEBPAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>UIT - RFID Manager</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }

    body {
      font-family: 'Segoe UI', Tahoma, sans-serif;
      min-height: 100vh; color: #fff;
      background-color: #0a1930;
      background-image: url('https://www.uit.edu.vn/_next/image?url=%2Fmedia%2Fimage_68800ba31d.png&w=1536&q=75');
      background-size: cover; background-position: center;
      background-repeat: no-repeat; background-attachment: fixed;
    }
    body::before {
      content: '';
      position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(10, 25, 60, 0.78);
      z-index: 0;
    }

    .container { max-width: 720px; margin: 0 auto; padding: 20px; position: relative; z-index: 1; }

    /* Header - Logo UIT */
    .header {
      text-align: center; margin-bottom: 24px;
      animation: fadeDown 0.6s ease-out;
    }
    @keyframes fadeDown { from { opacity:0; transform:translateY(-20px); } to { opacity:1; transform:translateY(0); } }

    .logo-text {
      display: inline-block; font-size: 2.2em; font-weight: 900;
      letter-spacing: 6px; margin-bottom: 6px; padding: 8px 18px;
      background: linear-gradient(135deg, #2f6bff, #60a0ff);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
      border: 3px solid rgba(47,107,255,0.5); border-radius: 12px;
      text-shadow: none;
    }
    .header h1 {
      font-size: 1.3em; font-weight: 700; letter-spacing: 1px;
      color: #fff;
    }
    .header .subtitle {
      font-size: 0.85em; color: rgba(255,255,255,0.65); margin-top: 2px;
    }

    /* Card chung */
    .card {
      background: rgba(20, 40, 80, 0.55);
      border: 1px solid rgba(47, 107, 255, 0.25);
      border-radius: 16px; padding: 24px; margin-bottom: 20px;
      backdrop-filter: blur(14px); -webkit-backdrop-filter: blur(14px);
      animation: fadeUp 0.5s ease-out both;
    }
    .card:nth-child(2) { animation-delay: 0.1s; }
    .card:nth-child(3) { animation-delay: 0.2s; }
    @keyframes fadeUp { from { opacity:0; transform:translateY(18px); } to { opacity:1; transform:translateY(0); } }

    .card-title {
      font-size: 1.05em; font-weight: 600; margin-bottom: 15px;
      color: #7eb4ff;
    }

    /* Khu vực quét thẻ */
    #scanArea {
      text-align: center; padding: 28px;
      border: 2px dashed rgba(47, 107, 255, 0.3); border-radius: 12px;
      transition: all 0.4s ease;
    }
    #scanArea.detected {
      border-color: #4caf50; background: rgba(76,175,80,0.1);
      animation: scanPulse 0.8s ease;
    }
    @keyframes scanPulse {
      0%,100% { box-shadow: 0 0 0 rgba(76,175,80,0); }
      50% { box-shadow: 0 0 25px rgba(76,175,80,0.3); }
    }
    .scan-icon { font-size: 2.2em; margin-bottom: 8px; }
    #scanUID {
      font-size: 1.9em; font-weight: 700; letter-spacing: 5px;
      color: #fff; margin: 8px 0;
      font-family: 'Courier New', monospace;
      text-shadow: 0 0 12px rgba(47,107,255,0.4);
    }
    #scanStatus { color: rgba(255,255,255,0.55); font-size: 0.88em; }

    /* Form thêm */
    #addForm { display: none; margin-top: 14px; }
    #addForm.show { display: block; animation: fadeUp 0.3s ease; }
    input[type="text"] {
      width: 100%; padding: 11px 15px;
      border: 1px solid rgba(47,107,255,0.3);
      border-radius: 10px; background: rgba(255,255,255,0.07);
      color: #fff; font-size: 0.95em; margin-bottom: 10px;
      outline: none; transition: border 0.3s;
    }
    input[type="text"]:focus { border-color: #2f6bff; box-shadow: 0 0 8px rgba(47,107,255,0.25); }
    input::placeholder { color: rgba(255,255,255,0.35); }

    /* Nút */
    .btn {
      padding: 10px 20px; border: none; border-radius: 10px;
      font-size: 0.92em; cursor: pointer; font-weight: 600;
      transition: all 0.25s; display: inline-block; margin: 3px;
    }
    .btn-add {
      background: linear-gradient(135deg, #2f6bff, #1a4fd6); color: #fff;
    }
    .btn-add:hover { transform: translateY(-2px); box-shadow: 0 4px 18px rgba(47,107,255,0.45); }
    .btn-del {
      background: rgba(220,53,69,0.85); color: #fff;
      padding: 6px 14px; font-size: 0.82em; border-radius: 8px;
    }
    .btn-del:hover { background: #dc3545; transform: translateY(-1px); }

    /* Bảng danh sách */
    table { width: 100%; border-collapse: collapse; }
    th {
      text-align: left; padding: 9px 12px; font-size: 0.78em;
      color: rgba(255,255,255,0.45); border-bottom: 1px solid rgba(47,107,255,0.15);
      text-transform: uppercase; letter-spacing: 1px;
    }
    td {
      padding: 11px 12px; border-bottom: 1px solid rgba(255,255,255,0.05);
      vertical-align: middle; font-size: 0.92em;
    }
    tr:hover td { background: rgba(47,107,255,0.06); }
    .uid-cell {
      font-family: 'Courier New', monospace; letter-spacing: 2px;
      color: #7eb4ff;
    }
    .empty-msg {
      text-align: center; padding: 28px; color: rgba(255,255,255,0.35);
      font-style: italic;
    }

    /* Toast */
    .toast {
      position: fixed; bottom: 28px; left: 50%; transform: translateX(-50%);
      background: rgba(47,107,255,0.92); color: #fff; padding: 11px 22px;
      border-radius: 10px; font-size: 0.88em; opacity: 0;
      transition: opacity 0.3s; pointer-events: none; z-index: 100;
      box-shadow: 0 4px 20px rgba(0,0,0,0.3);
    }
    .toast.show { opacity: 1; }

    /* Footer */
    .footer {
      text-align: center; padding: 18px 0 8px;
      color: rgba(255,255,255,0.3); font-size: 0.75em;
    }
  </style>
</head>
<body>
  <div class="container">

    <!-- HEADER -->
    <div class="header">
      <div class="logo-text">UIT</div>
      <h1>HE THONG KIEM SOAT RFID</h1>
      <div class="subtitle">Truong Dai Hoc Cong Nghe Thong Tin - DHQG HCM</div>
    </div>

    <!-- PHAN 1: QUET THE -->
    <div class="card">
      <div class="card-title">&#128225; Quet The</div>
      <div id="scanArea">
        <div class="scan-icon">&#128179;</div>
        <div id="scanUID">-- -- -- --</div>
        <div id="scanStatus">Dua the lai gan dau doc...</div>
      </div>
      <div id="addForm">
        <input type="text" id="userName" placeholder="Nhap ten nguoi dung cho the nay...">
        <button class="btn btn-add" onclick="addUser()">&#10004; Them vao danh sach</button>
      </div>
    </div>

    <!-- PHAN 2: DANH SACH -->
    <div class="card">
      <div class="card-title">&#128101; Danh Sach Nguoi Dung (<span id="userCount">0</span>)</div>
      <div id="userTable"></div>
    </div>

    <div class="footer">UIT RFID System &bull; ESP32</div>
  </div>

  <div class="toast" id="toast"></div>

  <script>
    let lastUID = '';
    let scannedUID = '';

    function showToast(msg) {
      const t = document.getElementById('toast');
      t.textContent = msg;
      t.classList.add('show');
      setTimeout(() => t.classList.remove('show'), 2500);
    }

    function loadUsers() {
      fetch('/list').then(r => r.json()).then(data => {
        document.getElementById('userCount').textContent = data.length;
        if (data.length === 0) {
          document.getElementById('userTable').innerHTML =
            '<div class="empty-msg">Chua co nguoi dung nao. Hay quet the de them!</div>';
          return;
        }
        let html = '<table><thead><tr><th>Ten</th><th>UID</th><th></th></tr></thead><tbody>';
        data.forEach((u, i) => {
          html += '<tr><td>' + u.name + '</td>';
          html += '<td class="uid-cell">' + u.uid + '</td>';
          html += '<td><button class="btn btn-del" onclick="delUser(' + i + ')">Xoa</button></td></tr>';
        });
        html += '</tbody></table>';
        document.getElementById('userTable').innerHTML = html;
      });
    }

    function checkScan() {
      fetch('/scan').then(r => r.json()).then(data => {
        if (data.uid && data.uid !== lastUID) {
          lastUID = data.uid;
          scannedUID = data.uid;
          document.getElementById('scanUID').textContent = data.uid;
          const area = document.getElementById('scanArea');
          area.classList.remove('detected');
          void area.offsetWidth;
          area.classList.add('detected');

          if (data.known) {
            document.getElementById('scanStatus').textContent =
              'Da nhan dien: ' + data.userName;
            document.getElementById('addForm').classList.remove('show');
          } else {
            document.getElementById('scanStatus').textContent =
              'The moi! Ban co muon them vao danh sach?';
            document.getElementById('addForm').classList.add('show');
            document.getElementById('userName').value = '';
            document.getElementById('userName').focus();
          }
          loadUsers();
        }
      });
    }

    function addUser() {
      const name = document.getElementById('userName').value.trim();
      if (!name) { showToast('Vui long nhap ten!'); return; }
      fetch('/add?name=' + encodeURIComponent(name) + '&uid=' + encodeURIComponent(scannedUID))
        .then(r => r.text()).then(() => {
          showToast('Da them: ' + name);
          document.getElementById('addForm').classList.remove('show');
          document.getElementById('scanStatus').textContent = 'Da them thanh cong!';
          loadUsers();
        });
    }

    function delUser(index) {
      if (!confirm('Ban co chac muon xoa nguoi dung nay?')) return;
      fetch('/delete?id=' + index).then(r => r.text()).then(() => {
        showToast('Da xoa nguoi dung!');
        loadUsers();
      });
    }

    loadUsers();
    setInterval(checkScan, 1500);
  </script>
</body>
</html>
)rawliteral";

#endif

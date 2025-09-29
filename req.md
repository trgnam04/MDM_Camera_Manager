Viết một process mô phỏng chức năng của Camera Manager [MDM], hiện thực bằng ngôn ngữ C/C++

- Đọc rule trong file profile.json, mẫu như sau:
{"policies": [ { "key":"connectivity.camera", "name": "Camera", "value":false } ] }

- Dựa vào value trong rule để quản lý camera: nếu value = true: luôn cho phép bật camera; nếu value = false: chỉ cho phép bật camera khi thoả điều kiện (có thể lọc theo app, user… -> phần này em thử research thêm xem ngta làm như nào, có thể thêm 1 trường whitelist trong json ở trên để chứa các app hoặc user được phép bật camera…). Nếu không thoả điều kiện thì không cho phép bật camera, đồng thời gửi cảnh báo đến server thông qua dbus
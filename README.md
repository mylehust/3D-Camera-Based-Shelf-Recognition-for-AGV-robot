# 3D-Camera-Based-Shelf-Recognition-for-AGV-robot
Sử dụng camera 3d D435 để nhận biết giá hàng cho robot

![image](https://github.com/user-attachments/assets/7af20924-0a4d-42ed-9402-0163d8566736)

Cụ thể quá trình được diễn ra như sau:

**Step 1:** Thu thập dữ liệu bằng camera D435f: Camera D435f được sử dụng để ghi nhận dữ liệu về hình ảnh và khoảng cách trong không gian ba chiều (3D). Nó có khả năng xác định vị trí các đối tượng (giá hàng) trong môi trường thực với độ chính xác cao thông qua việc đo khoảng cách từ camera đến các đối tượng và ma trận Intrinsics của nó. Thông qua đó, ta có thể ghi nhận được các thông tin như: tọa độ 3D 2 điểm marker, khoảng cách tới marker, khoảng cách của 2 marker với nhau, góc lệch…. Những dữ liệu này, đủ để Robot có thể đưa ra những quyết định di chuyển phù hợp với mục đích của mình.

**Step 2:** Xử lý dữ liệu trên Orange Pi 4: Để D435f có thể thực hiện nhiệm vụ của mình, cần phải sử dụng một kit nhúng - Orange Pi 4, một máy tính nhỏ gọn và mạnh mẽ. Từ đó D435f sẽ tính toán và xác định tọa độ X, Y, Z của các điểm quan trọng như các góc trái và phải của đối tượng.

**Step 3:** Định dạng dữ liệu thành JSON: Sau khi xử lý và lấy được thông tin từ D435f, Orange Pi 4 sẽ đóng gói thông tin này dưới dạng JSON (JavaScript Object Notation), một định dạng dữ liệu nhẹ và dễ dàng trao đổi giữa các hệ thống. Trong JSON này, bao gồm các tọa độ của các điểm, khoảng cách giữa các góc, và trạng thái phát hiện (dollyFound) của đối tượng.

**Step 4:** Truyền dữ liệu qua TCP/IP: TCP/IP là một giao thức mạng chuẩn, cho phép truyền thông tin một cách ổn định và hiệu quả giữa các thiết bị. Dữ liệu JSON sau đó được truyền qua giao thức TCP/IP từ Pi đến một máy tính hoặc thiết bị khác để hiển thị kết quả một cách trực quan hơn.

**Step 5:** Hiển thị kết quả trên giao diện người dùng (UI): Cuối cùng, dữ liệu JSON được nhận và hiển thị trên giao diện người dùng (UI) được phát triển trong QT Creator. Giao diện này cung cấp các thông tin trực quan như khoảng cách giữa các góc, vị trí các góc trong không gian, và trạng thái phát hiện của đối tượng. Người dùng có thể theo dõi và tương tác với các thông số này trực tiếp trên giao diện, do đó có thể giám sát, phân tích dữ liệu, thậm chí là điều khiển đưa ra các lệnh phù hợp với các tiền kết quả.

![image](https://github.com/user-attachments/assets/835b8676-9166-4cbc-b8ab-318f81c63810)


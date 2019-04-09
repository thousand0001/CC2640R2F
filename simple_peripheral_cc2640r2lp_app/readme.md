TI CC2640 藍牙開發重點 
使用CC2640R2 LACUCHXL開發板
登錄TI網站的開發資源https://dev.ti.com/，登錄帳號密碼
另一個頁面開啟線上開發工具ccs
插入開發板，在資源網站左邊會有顯示，找到開發板，點進去資源的地方，叫出simple_peripheral的專案，網頁右上角點擊圖案，將專案匯入CCS
開啟另一個說明網站Bluetooth Low Energy and TI Drivers   http://dev.ti.com/tirex/content/simplelink_academy_cc2640r2sdk_2_40_03_00/modules/blestack/ble_posix/ble_01_posix.html，在CCS中開使依照說明編輯檔案，
TI的藍牙芯片內容分兩塊，所以要搞定MCU含低功耗藍牙BLE這兩塊，從說明的task3開始就行
依照task3建立myThread.c的線程task，這一塊主要是MCU對周邊的操控，讓Simple Peripheral和myThread兩個程式可以同步進行，Simple Peripheral主要控制藍牙堆疊（藍牙棧），myThread跑MCU的周邊，兩個程序用全域變數來溝通。
依照說明在main()主函數裡面加入myThread()線程。
重點來了，使用TI的工具來產生藍牙服務，這個工具在這個網頁的最下端，設立服務的uuid等等，然後增加屬性，會產生三個檔案，前兩個分別是.c和.h檔案，將這兩個檔案放入專案中的profile資料夾內。

第三個檔案名稱為application-snippets.c，這不是一個完整的檔案，只是告訴你如何在原來的SimplePeripheral.c檔案中增加程式碼，來執行新增的服務。
生成功具有另外比較詳細的說明：http://dev.ti.com/tirex/content/simplelink_academy_cc2640r2sdk_2_40_03_00/modules/blestack/ble_01_custom_profile/ble_01_custom_profile.html
要執行藍牙上傳，就要使用simple_peripheral.c，要控制周邊，就要用myThread，這個原則之下，可以使用全域變數來傳遞變數內容，
要週期性的藍牙上傳到host，可以使用simple_peripheral.c裡面的SimplePeripheral_performPeriodicTask()，把要上傳的API：MyData_SetParameter(MYDATA_DATA_ID, MYDATA_DATA_LEN, &myData);放在這裡面。
要在TI工具中使用printf功能？要改成Display_printf如下
#include <ti/display/Display.h>
在main中，循環之前
Display_Handle display;
display = Display_open(Display_Type_UART, NULL);
…
開始循環顯示
Display_printf(display, 0, 0, "Count = %d and it's address is 0x%x\n", count, &count);
這兩個參數0,0在UART中沒有意義，不用管他。參考下列網址：
http://dev.ti.com/tirex/content/simplelink_academy_cc13x2sdk_2_10_02_10/modules/debug/debugging_output/debugging_output.html


GATT 事務是建立在嵌套的Profiles, Services 和 Characteristics之上的的，如右圖所示，Profile 並不是實際存在於 BLE 外設上的，他只是被SIG這個機構定義，參考原文網址：https://read01.com/BjJNz7.html


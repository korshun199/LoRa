@echo off
chcp 65001 >nul
title CHUVAK LoRa Monitor

echo ==========================================
echo        CHUVAK LoRa Monitor for Windows
echo ==========================================
echo.
echo Подключи плату ЧУВАК в USB.
echo Если уже подключена - просто жди.
echo.

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
"$baud=115200; ^
Write-Host 'Ищу COM-порт...'; ^
$ports=Get-CimInstance Win32_SerialPort | Select-Object DeviceID,Name; ^
if(-not $ports){ Write-Host 'COM-порт не найден. Переткни плату. Потому что Windows, конечно.'; pause; exit 1 }; ^
$ports | ForEach-Object { Write-Host ($_.DeviceID + '  ' + $_.Name) }; ^
$port=($ports | Select-Object -First 1).DeviceID; ^
Write-Host ''; ^
Write-Host ('Открываю ' + $port + ' на 115200'); ^
Write-Host 'Закрыть окно = выйти'; ^
Write-Host ''; ^
Add-Type -AssemblyName System.IO.Ports; ^
while($true){ ^
  try{ ^
    $s=New-Object System.IO.Ports.SerialPort $port,$baud,'None',8,'One'; ^
    $s.Encoding=[System.Text.Encoding]::UTF8; ^
    $s.ReadTimeout=1000; ^
    $s.Open(); ^
    Write-Host ('[OK] Слушаю ' + $port); ^
    Start-Sleep -Seconds 1; ^
    while($s.IsOpen){ ^
      try{ $line=$s.ReadLine(); if($line){ Write-Host $line.Trim() } } ^
      catch [System.TimeoutException]{} ^
    } ^
  } catch { ^
    Write-Host ('[reconnect] ' + $_.Exception.Message); ^
    Start-Sleep -Seconds 2; ^
  } ^
}"

pause

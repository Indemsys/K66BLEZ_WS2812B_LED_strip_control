unit Main;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes,
  System.Variants,
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs, FMX.TabControl,
  FMX.Media, FMX.Objects, FMX.Consts,
  System.Bluetooth, FMX.StdCtrls, System.Bluetooth.Components,
  FMX.Controls.Presentation, FMX.ListBox, FMX.Layouts, FMX.ScrollBox, FMX.Memo;

const
  Cmd_service_UUID: TBluetoothUUID = '{37FE7100-FB31-49E6-8389-72414CEE1597}';
  Cmd_charact_write_UUID
    : TBluetoothUUID = '{37FE7101-FB31-49E6-8389-72414CEE1597}';
  // Характеристика для отправки команд
  Cmd_charact_read_UUID
    : TBluetoothUUID = '{37FE7102-FB31-49E6-8389-72414CEE1597}';
  // Характеристика для чтения состояния и подтверждения выполнения команд
  TEST_CHAR_VAL_MAX_SZ = 20;

  // Коды ответов
  REPLY_DEVICE_READY_FOR_CMD = $AA00; // Устройство готово к обмену
  REPLY_FILE_PREPARED = $AA01; // Файл подготовлен
  REPLY_FILE_ERROR = $AA03; // Ошибка открытиия файла в дивайсе
  REPLY_PLAYING_END = $AA04; // Завершено воспроизведение файла в дивайсе
  REPLY_CMD_ERROR = $01010101; // Ошибка команды

  // Коды команд
  CMD_PLAY_FILE = $0002;
  CMD_STOP_PLAY = $0003;

type
  TfrmMain = class(TForm)
    BLE: TBluetoothLE;
    lblDeviceStatus: TLabel;
    btnStartPlay: TCornerButton;
    btnStopPlay: TCornerButton;
    logMemo: TMemo;
    procedure bPlayClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure bStopClick(Sender: TObject);
    procedure BLEDiscoverLEDevice(const Sender: TObject;
      const ADevice: TBluetoothLEDevice; Rssi: Integer;
      const ScanResponse: TScanResponse);
    procedure BLEEndDiscoverDevices(const Sender: TObject;
      const ADeviceList: TBluetoothLEDeviceList);
    procedure BLEServicesDiscovered(const Sender: TObject;
      const AServiceList: TBluetoothGattServiceList);
    procedure BLEConnectedDevice(const Sender: TObject;
      const ADevice: TBluetoothLEDevice);
    procedure BLEDisconnectDevice(const Sender: TObject;
      const ADevice: TBluetoothLEDevice);
    procedure BLECharacteristicRead(const Sender: TObject;
      const ACharacteristic: TBluetoothGattCharacteristic;
      AGattStatus: TBluetoothGattStatus);
    procedure BLEConnect(Sender: TObject);
    procedure BLEDisconnect(Sender: TObject);
    procedure BLECharacteristicWrite(const Sender: TObject;
      const ACharacteristic: TBluetoothGattCharacteristic;
      AGattStatus: TBluetoothGattStatus);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private declarations }
    BLEDevice: TBluetoothLEDevice;
    BLEService: TBluetoothGattService;
    BLEReadCmd: TBluetoothGattCharacteristic;
    BLEWriteCmd: TBluetoothGattCharacteristic;
    device_ready: boolean;
    FLibraryPath: string;
    function GetPathWithVideo: string;
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation

uses
  System.IOUtils;

{$R *.fmx}

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.FormCreate(Sender: TObject);
begin
  FLibraryPath := GetPathWithVideo;
  BLEDevice := Nil;
  device_ready := False;
  BLE.CurrentManager.StartDiscovery(1000, Nil, True);
end;

{ -----------------------------------------------------------------------------


  Arguments: None
  Result:    string
  ----------------------------------------------------------------------------- }
function TfrmMain.GetPathWithVideo: string;
begin
  case TOSVersion.Platform of
    TOSVersion.TPlatform.pfWindows:
      Result := 'Video\';
    TOSVersion.TPlatform.pfMacOS:
      Result := TPath.GetFullPath('../Resources/StartUp');
    TOSVersion.TPlatform.pfiOS, TOSVersion.TPlatform.pfAndroid:
      Result := TPath.GetDocumentsPath;
    TOSVersion.TPlatform.pfWinRT, TOSVersion.TPlatform.pfLinux:
      raise Exception.Create('Unexpected platform');
  end;
end;


{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.bPlayClick(Sender: TObject);
begin
  // Подпишемся на эту характеристику чтобы получать ответы на команды
  BLE.SubscribeToCharacteristic(BLEDevice, BLEReadCmd);

  // Начинаем с отправки имени файла устройству
  BLEWriteCmd.SetValueAsString('');
  BLEWriteCmd.SetValueAsUInt32(CMD_PLAY_FILE);
  BLEDevice.WriteCharacteristic(BLEWriteCmd);
end;

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.bStopClick(Sender: TObject);
begin
  BLEWriteCmd.SetValueAsString('');
  BLEWriteCmd.SetValueAsUInt32(CMD_STOP_PLAY);
  BLEDevice.WriteCharacteristic(BLEWriteCmd);
end;


{ -----------------------------------------------------------------------------
  Событие обнаружения нового устройства

  Arguments: const Sender: TObject; const ADevice: TBluetoothLEDevice; Rssi: Integer; const ScanResponse: TScanResponse
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEDiscoverLEDevice(const Sender: TObject;
  const ADevice: TBluetoothLEDevice; Rssi: Integer;
  const ScanResponse: TScanResponse);
begin
  if ADevice.DeviceName = 'K66BLEZ' then
  begin
    lblDeviceStatus.Text := 'Controller found. Servise searching...';
    BLEDevice := ADevice;
    BLE.CancelDiscovery;
    Application.ProcessMessages;

    // Синхролнный вызов обнаружения сервисов у устройства
    BLEDevice.DiscoverServices;

  end;

end;

{ -----------------------------------------------------------------------------
  Событие после завершения поиска устройств

  Arguments: const Sender: TObject; const ADeviceList: TBluetoothLEDeviceList
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEEndDiscoverDevices(const Sender: TObject;
  const ADeviceList: TBluetoothLEDeviceList);
begin
  if BLEDevice = Nil then
    lblDeviceStatus.Text := 'Controller not found. ' +
      ADeviceList.Count.ToString;
end;

{ -----------------------------------------------------------------------------
  Событие после обнаружения всех сервисов у устройства

  Arguments: const Sender: TObject; const AServiceList: TBluetoothGattServiceList
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEServicesDiscovered(const Sender: TObject;
  const AServiceList: TBluetoothGattServiceList);
var
  serv_indx: Integer;
  chrkt_indx: Integer;
begin
  for serv_indx := 0 to AServiceList.Count - 1 do
  begin
    if AServiceList[serv_indx].UUID = Cmd_service_UUID then
    begin
      // Найден интересующий нас сервис
      BLEService := AServiceList[serv_indx];
      // Найдем интересующие нас характеристики
      BLEReadCmd := Nil;
      BLEWriteCmd := Nil;
      for chrkt_indx := 0 to BLEService.Characteristics.Count - 1 do
      begin
        if BLEService.Characteristics[chrkt_indx].UUID = Cmd_charact_write_UUID
        then
          BLEWriteCmd := BLEService.Characteristics[chrkt_indx];
        if BLEService.Characteristics[chrkt_indx].UUID = Cmd_charact_read_UUID
        then
          BLEReadCmd := BLEService.Characteristics[chrkt_indx];

      end;

      if (BLEReadCmd <> Nil) and (BLEWriteCmd <> Nil) then
      begin
        lblDeviceStatus.Text := 'Сontroller is found.';
        Application.ProcessMessages;
        // Запрашиваем состояние и одновременно с этои устанавливается соединение
        if BLEDevice.ReadCharacteristic(BLEReadCmd) = False then
        begin
          logMemo.Lines.Add('Attempt to read characteristic fault!');
        end;
      end
      else
      begin
        lblDeviceStatus.Text := 'Controller haven''t required services.'
      end;

    end;

  end;

end;

{ -----------------------------------------------------------------------------
  CallBack вызываемый после чтения характеристики из устройства


  Arguments: const Sender: TObject; const ACharacteristic: TBluetoothGattCharacteristic; AGattStatus: TBluetoothGattStatus
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLECharacteristicRead(const Sender: TObject;
  const ACharacteristic: TBluetoothGattCharacteristic;
  AGattStatus: TBluetoothGattStatus);
begin
  if ACharacteristic = BLEReadCmd then
  begin
    if ACharacteristic.GetValueAsUInt32 = REPLY_DEVICE_READY_FOR_CMD then
    begin
      device_ready := True;
      lblDeviceStatus.Text := 'Application ready.';
    end
    else if ACharacteristic.GetValueAsUInt32 = REPLY_FILE_PREPARED then
    begin
      // Файл подготовлен
      // Высылаем команду начать воспроизведение и начинаем воспроизведение у себя
      BLEWriteCmd.SetValueAsString('');
      // Обнуляем характеритсику прежде чем присваиваем. Поскольку для на характеристики может остаться от предыдущей строки в нее записанной
      BLEWriteCmd.SetValueAsUInt32(CMD_PLAY_FILE);
      BLEDevice.WriteCharacteristic(BLEWriteCmd);
      // MediaPlayer1.Play;    // Похоже здесь нельзя запускать плейер, программа зависает
    end
    else if ACharacteristic.GetValueAsUInt32 = REPLY_FILE_ERROR then
    begin
      logMemo.Lines.Add('Device can''t to find or to open file!');
    end
    else if ACharacteristic.GetValueAsUInt32 = REPLY_PLAYING_END then
    begin
      logMemo.Lines.Add('Stop!');
    end
    else if ACharacteristic.GetValueAsUInt32 = REPLY_CMD_ERROR then
    begin
      logMemo.Lines.Add('Command error!');
    end;

  end;

end;

{ -----------------------------------------------------------------------------
  CallBack вызываемый после записи характеристики в устройство

  Arguments: const Sender: TObject; const ACharacteristic: TBluetoothGattCharacteristic; AGattStatus: TBluetoothGattStatus
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLECharacteristicWrite(const Sender: TObject;
  const ACharacteristic: TBluetoothGattCharacteristic;
  AGattStatus: TBluetoothGattStatus);
begin
  if AGattStatus <> TBluetoothGattStatus.Success then
  begin
  end;

end;

{ -----------------------------------------------------------------------------


  Arguments: const Sender: TObject; const ADevice: TBluetoothLEDevice
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEConnectedDevice(const Sender: TObject;
  const ADevice: TBluetoothLEDevice);
begin
  lblDeviceStatus.TextSettings.FontColor := TAlphaColorRec.Green;
end;

{ -----------------------------------------------------------------------------


  Arguments: const Sender: TObject; const ADevice: TBluetoothLEDevice
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEDisconnectDevice(const Sender: TObject;
  const ADevice: TBluetoothLEDevice);
begin
  device_ready := False;
  lblDeviceStatus.TextSettings.FontColor := TAlphaColorRec.Red;
  lblDeviceStatus.Text := 'Controller disconnected';
end;

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEConnect(Sender: TObject);
begin
  lblDeviceStatus.TextSettings.FontColor := TAlphaColorRec.Green;
end;

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.BLEDisconnect(Sender: TObject);
begin
  device_ready := False;
  lblDeviceStatus.TextSettings.FontColor := TAlphaColorRec.Red;
  lblDeviceStatus.Text := 'Controller disconnected';
end;

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject; var Action: TCloseAction
  Result:    None
  ----------------------------------------------------------------------------- }
procedure TfrmMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  bStopClick(self);
end;

{ -----------------------------------------------------------------------------


  Arguments: Sender: TObject
  Result:    None
  ----------------------------------------------------------------------------- }
end.

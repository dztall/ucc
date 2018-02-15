object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'MiniAPI shader viewer'
  ClientHeight = 400
  ClientWidth = 400
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object paHeader: TPanel
    Left = 0
    Top = 0
    Width = 400
    Height = 27
    Align = alTop
    AutoSize = True
    BevelOuter = bvNone
    TabOrder = 0
    object cbShaders: TComboBox
      AlignWithMargins = True
      Left = 3
      Top = 3
      Width = 356
      Height = 21
      Align = alClient
      TabOrder = 0
      OnChange = cbShadersChange
    end
    object btChangeDir: TButton
      AlignWithMargins = True
      Left = 365
      Top = 3
      Width = 32
      Height = 21
      Align = alRight
      Caption = '...'
      TabOrder = 1
      OnClick = btChangeDirClick
    end
  end
  object paView: TPanel
    Left = 0
    Top = 27
    Width = 400
    Height = 373
    Cursor = crSizeAll
    Align = alClient
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 1
    OnMouseMove = paViewMouseMove
  end
  object odOpen: TOpenDialog
    DefaultExt = '.shader'
    Filter = 'Shader files|*.shader'
    Options = [ofHideReadOnly, ofNoValidate, ofEnableSizing]
    Left = 360
    Top = 43
  end
end

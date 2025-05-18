# ESP32 - App Kommunikationsprotokoll


### ESP -> App - Ready

```json
{
    "state": "ready"
}
```

### App -> ESP - Configuration

```json
{
    "event": "configuration",
    "data": [106,19,19,206,193,30]
}
```

### ESP -> App - Configuration OK

```json
{
    "state": "disconnected"
}
```

### ESP -> App - Connected

```json
{
    "state": "connected"
}
```

### ESP -> App - Data

```json
{
    "event": "telemetry",
    "data": [...]
}
```

### 

```json
{
    
}
```
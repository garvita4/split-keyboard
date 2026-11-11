#include <Arduino.h>
#pragma once
#include <Arduino.h>
#if defined(SIDE_LEFT) == defined(SIDE_RIGHT)
#error "Build with exactly one of -DSIDE_LEFT / -DSIDE_RIGHT (use the PlatformIO envs 'left' and 'right')."
#endif
constexpr uint8_t NUM_ROWS = 4;
constexpr uint8_t NUM_COLS = 6;
constexpr uint8_t NUM_KEYS = NUM_ROWS * NUM_COLS;  
constexpr uint8_t ROW_PINS[NUM_ROWS] = {4, 5, 6, 7};
constexpr uint8_t COL_PINS[NUM_COLS] = {15, 16, 17, 18, 21, 38};
constexpr bool keyExists(uint8_t row, uint8_t col) { return !(row == 3 && col < 3); }
constexpr uint8_t ENC_A_PIN = 10;
constexpr uint8_t ENC_B_PIN = 11;
constexpr uint8_t ENC_SW_PIN = 12;
constexpr int8_t ENC_DIRECTION = 1;  
constexpr uint8_t BACKLIGHT_PIN = 13;
constexpr uint8_t BACKLIGHT_LEVELS[] = {0, 16, 48, 112, 255};  
constexpr uint8_t BACKLIGHT_NUM_LEVELS = sizeof(BACKLIGHT_LEVELS);
constexpr uint8_t BACKLIGHT_DEFAULT_LEVEL = 2;
constexpr uint32_t BACKLIGHT_IDLE_MS = 60UL * 1000UL;  
constexpr uint32_t DEBOUNCE_MS = 5;         
constexpr uint32_t HEARTBEAT_MS = 50;       
constexpr uint32_t REMOTE_TIMEOUT_MS = 250; 
constexpr uint32_t BACKLIGHT_SYNC_MS = 500; 
constexpr uint8_t SPLIT_WIFI_CHANNEL = 1;
#ifdef DEBUG_KEYS
#define LOG_KEY(...) Serial.printf(__VA_ARGS__)
#else
#define LOG_KEY(...) ((void)0)
#endif
#pragma once
#include <BleKeyboard.h>
constexpr uint16_t KC_TRNS = 0;
constexpr uint16_t KC_MO1 = 0x100;
constexpr uint8_t NUM_LAYERS = 2;
const uint16_t KEYMAP[NUM_LAYERS][2][NUM_ROWS][NUM_COLS] = {
    {
        {
            {KEY_TAB,        'q', 'w', 'e', 'r', 't'},
            {KEY_LEFT_CTRL,  'a', 's', 'd', 'f', 'g'},
            {KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b'},
            {0, 0, 0,        KEY_LEFT_ALT, KC_MO1, ' '},
        },
        {
            {KEY_BACKSPACE,   'p', 'o', 'i', 'u', 'y'},
            {'\'',            ';', 'l', 'k', 'j', 'h'},
            {KEY_RIGHT_SHIFT, '/', '.', ',', 'm', 'n'},
            {0, 0, 0,         KEY_RIGHT_ALT, KC_MO1, KEY_RETURN},
        },
    },
    {
        {
            {'`',     '1',        '2',        '3',        '4',        '5'},
            {KC_TRNS, KEY_F1,     KEY_F2,     KEY_F3,     KEY_F4,     KEY_F5},
            {KC_TRNS, KEY_F6,     KEY_F7,     KEY_F8,     KEY_F9,     KEY_F10},
            {0, 0, 0, KC_TRNS,    KC_TRNS,    KC_TRNS},
        },
        {
            {KEY_DELETE, '0', '9',            '8',          '7',            '6'},
            {'=',        '-', KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW},
            {KC_TRNS,    '\\', ']',           '[',          KEY_F12,        KEY_F11},
            {0, 0, 0,    KC_TRNS, KC_TRNS,    KC_TRNS},
        },
    },
};
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
namespace proto {
constexpr uint8_t MAGIC_STATE = 0xA5;      
constexpr uint8_t MAGIC_BACKLIGHT = 0x5A;  
constexpr uint8_t NET_KEY = 0x3C;
constexpr uint32_t VALID_KEY_MASK = 0x00FFFFFFu & ~(0x7u << 18);
inline uint8_t crc8(const uint8_t* d, size_t n) {
    uint8_t c = NET_KEY;
    for (size_t i = 0; i < n; i++) {
        c ^= d[i];
        for (int b = 0; b < 8; b++) c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x07) : (uint8_t)(c << 1);
    }
    return c;
}
struct __attribute__((packed)) StatePacket {
    uint8_t magic;
    uint8_t seq;
    uint8_t keys[4];  
    uint8_t enc;      
    uint8_t flags;    
    uint8_t crc;
};
struct __attribute__((packed)) BacklightPacket {
    uint8_t magic;
    uint8_t seq;
    uint8_t duty;
    uint8_t crc;
};
inline void encodeState(StatePacket& p, uint8_t seq, uint32_t keys, uint8_t enc, uint8_t flags) {
    p.magic = MAGIC_STATE;
    p.seq = seq;
    for (int i = 0; i < 4; i++) p.keys[i] = (uint8_t)((keys >> (8 * i)) & 0xFF);
    p.enc = enc;
    p.flags = flags;
    p.crc = crc8(reinterpret_cast<const uint8_t*>(&p), sizeof(p) - 1);
}
inline bool decodeState(const uint8_t* data, int len, StatePacket& out) {
    if (len != (int)sizeof(StatePacket)) return false;
    memcpy(&out, data, sizeof(out));
    if (out.magic != MAGIC_STATE) return false;
    return out.crc == crc8(data, sizeof(out) - 1);
}
inline uint32_t keysOf(const StatePacket& p) {
    uint32_t k = 0;
    for (int i = 0; i < 4; i++) k |= (uint32_t)p.keys[i] << (8 * i);
    return k & VALID_KEY_MASK;
}
inline void encodeBacklight(BacklightPacket& p, uint8_t seq, uint8_t duty) {
    p.magic = MAGIC_BACKLIGHT;
    p.seq = seq;
    p.duty = duty;
    p.crc = crc8(reinterpret_cast<const uint8_t*>(&p), sizeof(p) - 1);
}
inline bool decodeBacklight(const uint8_t* data, int len, BacklightPacket& out) {
    if (len != (int)sizeof(BacklightPacket)) return false;
    memcpy(&out, data, sizeof(out));
    if (out.magic != MAGIC_BACKLIGHT) return false;
    return out.crc == crc8(data, sizeof(out) - 1);
}
inline bool seqNewer(uint8_t seq, uint8_t last) { return (int8_t)(seq - last) > 0; }
}  
#pragma once
#include <stdint.h>
namespace encoder_table {
constexpr uint8_t DIR_NONE = 0x0;
constexpr uint8_t DIR_CW = 0x10;
constexpr uint8_t DIR_CCW = 0x20;
constexpr uint8_t R_START = 0x0;
constexpr uint8_t R_CW_FINAL = 0x1;
constexpr uint8_t R_CW_BEGIN = 0x2;
constexpr uint8_t R_CW_NEXT = 0x3;
constexpr uint8_t R_CCW_BEGIN = 0x4;
constexpr uint8_t R_CCW_FINAL = 0x5;
constexpr uint8_t R_CCW_NEXT = 0x6;
constexpr uint8_t TABLE[7][4] = {
    {R_START, R_CW_BEGIN, R_CCW_BEGIN, R_START},                         
    {R_CW_NEXT, R_START, R_CW_FINAL, (uint8_t)(R_START | DIR_CW)},       
    {R_CW_NEXT, R_CW_BEGIN, R_START, R_START},                           
    {R_CW_NEXT, R_CW_BEGIN, R_CW_FINAL, R_START},                        
    {R_CCW_NEXT, R_START, R_CCW_BEGIN, R_START},                         
    {R_CCW_NEXT, R_CCW_FINAL, R_START, (uint8_t)(R_START | DIR_CCW)},    
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},                     
};
inline uint8_t step(uint8_t state, uint8_t pinstate) { return TABLE[state & 0xF][pinstate & 0x3]; }
}  
#pragma once
#include <Arduino.h>
class Matrix {
public:
    void begin();
    bool scan();
    uint32_t state() const { return state_; }
private:
    uint32_t state_ = 0;
    uint32_t lastChangeMs_[NUM_KEYS] = {0};
};
#pragma once
#include <Arduino.h>
struct RemoteState {
    uint32_t keys;     
    uint8_t encCount;  
    bool encButton;    
};
bool splitBegin();
void splitSendState(uint32_t keys, uint8_t encCount, bool encButton);
bool splitTakeBacklight(uint8_t &duty);
bool splitGetRemote(RemoteState &out, uint32_t &ageMs);
void splitSendBacklight(uint8_t duty);
#pragma once
#include <Arduino.h>
class Encoder {
public:
    void begin();
    void update();
    uint8_t counter() const { return counter_; }
    bool button() const { return button_; }
private:
    uint8_t state_ = 0;
    uint8_t counter_ = 0;
    bool button_ = false;
    uint32_t buttonChangeMs_ = 0;
};
#pragma once
#include <Arduino.h>
namespace backlight {
void begin();
void setDuty(uint8_t duty);  
}  
void Matrix::begin() {
    for (uint8_t r = 0; r < NUM_ROWS; r++) {
        pinMode(ROW_PINS[r], OUTPUT);
        digitalWrite(ROW_PINS[r], HIGH);
    }
    for (uint8_t c = 0; c < NUM_COLS; c++) pinMode(COL_PINS[c], INPUT_PULLUP);
}
bool Matrix::scan() {
    const uint32_t now = millis();
    bool changed = false;
    for (uint8_t r = 0; r < NUM_ROWS; r++) {
        digitalWrite(ROW_PINS[r], LOW);
        delayMicroseconds(20);  
        for (uint8_t c = 0; c < NUM_COLS; c++) {
            if (!keyExists(r, c)) continue;
            const uint8_t idx = r * NUM_COLS + c;
            const bool raw = (digitalRead(COL_PINS[c]) == LOW);
            const bool cur = (state_ >> idx) & 1u;
            if (raw != cur && (now - lastChangeMs_[idx]) >= DEBOUNCE_MS) {
                state_ ^= (1u << idx);
                lastChangeMs_[idx] = now;
                changed = true;
            }
        }
        digitalWrite(ROW_PINS[r], HIGH);
    }
    return changed;
}
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
namespace {
const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
RemoteState g_remote = {0, 0, false};
uint32_t g_remoteMs = 0;
bool g_remoteValid = false;
uint8_t g_remoteSeq = 0;
uint8_t g_backlightDuty = 0;
bool g_backlightNew = false;
uint8_t g_backlightSeq = 0;
bool g_backlightSeqValid = false;
uint8_t g_txSeq = 0;
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
    (void)mac;
#ifdef SIDE_LEFT
    proto::StatePacket p;
    if (!proto::decodeState(data, len, p)) return;
    const uint32_t now = millis();
    portENTER_CRITICAL(&g_mux);
    if (!g_remoteValid || proto::seqNewer(p.seq, g_remoteSeq) || (now - g_remoteMs) > 300) {
        g_remote.keys = proto::keysOf(p);
        g_remote.encCount = p.enc;
        g_remote.encButton = (p.flags & 1) != 0;
        g_remoteSeq = p.seq;
        g_remoteMs = now;
        g_remoteValid = true;
    }
    portEXIT_CRITICAL(&g_mux);
#else
    proto::BacklightPacket p;
    if (!proto::decodeBacklight(data, len, p)) return;
    portENTER_CRITICAL(&g_mux);
    if (!g_backlightSeqValid || proto::seqNewer(p.seq, g_backlightSeq)) {
        g_backlightDuty = p.duty;
        g_backlightSeq = p.seq;
        g_backlightSeqValid = true;
        g_backlightNew = true;
    }
    portEXIT_CRITICAL(&g_mux);
#endif
}
}  
bool splitBegin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(SPLIT_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) return false;
    esp_now_register_recv_cb(onRecv);
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, BROADCAST_MAC, 6);
    peer.channel = SPLIT_WIFI_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}
void splitSendState(uint32_t keys, uint8_t encCount, bool encButton) {
    proto::StatePacket p;
    proto::encodeState(p, g_txSeq++, keys, encCount, encButton ? 1 : 0);
    esp_now_send(BROADCAST_MAC, reinterpret_cast<const uint8_t *>(&p), sizeof(p));
}
bool splitTakeBacklight(uint8_t &duty) {
    bool fresh;
    portENTER_CRITICAL(&g_mux);
    fresh = g_backlightNew;
    duty = g_backlightDuty;
    g_backlightNew = false;
    portEXIT_CRITICAL(&g_mux);
    return fresh;
}
bool splitGetRemote(RemoteState &out, uint32_t &ageMs) {
    bool valid;
    uint32_t stamp;
    portENTER_CRITICAL(&g_mux);
    valid = g_remoteValid;
    out = g_remote;
    stamp = g_remoteMs;
    portEXIT_CRITICAL(&g_mux);
    ageMs = valid ? (millis() - stamp) : 0xFFFFFFFFu;
    return valid;
}
void splitSendBacklight(uint8_t duty) {
    proto::BacklightPacket p;
    proto::encodeBacklight(p, g_txSeq++, duty);
    esp_now_send(BROADCAST_MAC, reinterpret_cast<const uint8_t *>(&p), sizeof(p));
}
void Encoder::begin() {
    pinMode(ENC_A_PIN, INPUT_PULLUP);
    pinMode(ENC_B_PIN, INPUT_PULLUP);
    pinMode(ENC_SW_PIN, INPUT_PULLUP);
}
void Encoder::update() {
    const uint8_t pins = (uint8_t)((digitalRead(ENC_B_PIN) << 1) | digitalRead(ENC_A_PIN));
    state_ = encoder_table::step(state_, pins);
    const uint8_t dir = state_ & 0x30;
    if (dir == encoder_table::DIR_CW) counter_ = (uint8_t)(counter_ + ENC_DIRECTION);
    else if (dir == encoder_table::DIR_CCW) counter_ = (uint8_t)(counter_ - ENC_DIRECTION);
    const uint32_t now = millis();
    const bool raw = (digitalRead(ENC_SW_PIN) == LOW);
    if (raw != button_ && (now - buttonChangeMs_) >= 10) {
        button_ = raw;
        buttonChangeMs_ = now;
    }
}
namespace {
constexpr uint8_t LEDC_CHANNEL = 0;
constexpr uint32_t PWM_FREQ_HZ = 20000;  
constexpr uint8_t PWM_BITS = 8;
}  
namespace backlight {
void begin() {
    ledcSetup(LEDC_CHANNEL, PWM_FREQ_HZ, PWM_BITS);
    ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);
}
void setDuty(uint8_t duty) { ledcWrite(LEDC_CHANNEL, duty); }
}  
#include <Arduino.h>
#include <Preferences.h>
#ifdef SIDE_LEFT
#include <BleKeyboard.h>
#endif
static Matrix matrix;
static Encoder encoder;
#ifdef SIDE_LEFT
static BleKeyboard ble("Split", "Garvita", 100);  
static Preferences prefs;
static uint16_t pressedCode[2][NUM_KEYS];  
static uint32_t prevKeys[2] = {0, 0};
static uint8_t layerHolds = 0;
static uint8_t brightnessLevel = BACKLIGHT_DEFAULT_LEVEL;
static uint8_t currentDuty = 0xFF;  
static uint8_t lastSentDuty = 0xFF;
static uint32_t lastBacklightSendMs = 0;
static uint32_t lastActivityMs = 0;
static uint8_t lastLocalEnc = 0;
static uint8_t lastRemoteEnc = 0;
static bool remoteEncInit = false;
static bool lastLocalBtn = false;
static bool lastRemoteBtn = false;
static bool wasConnected = false;
static bool wasRemoteUp = false;
static void markActive() { lastActivityMs = millis(); }
static uint16_t lookup(uint8_t side, uint8_t r, uint8_t c) {
    const uint8_t layer = layerHolds > 0 ? 1 : 0;
    uint16_t code = KEYMAP[layer][side][r][c];
    if (code == KC_TRNS && layer > 0) code = KEYMAP[0][side][r][c];
    return code;
}
static void keyEvent(uint8_t side, uint8_t idx, bool pressed) {
    const uint8_t r = idx / NUM_COLS, c = idx % NUM_COLS;
    markActive();
    if (pressed) {
        const uint16_t code = lookup(side, r, c);
        pressedCode[side][idx] = code;
        LOG_KEY("press   side=%u r=%u c=%u code=0x%03X\n", side, r, c, code);
        if (code == 0) return;
        if (code == KC_MO1) { layerHolds++; return; }
        if (ble.isConnected()) ble.press((uint8_t)code);
    } else {
        const uint16_t code = pressedCode[side][idx];
        pressedCode[side][idx] = 0;
        LOG_KEY("release side=%u r=%u c=%u code=0x%03X\n", side, r, c, code);
        if (code == 0) return;
        if (code == KC_MO1) { if (layerHolds) layerHolds--; return; }
        if (ble.isConnected()) ble.release((uint8_t)code);
    }
}
static void processKeys(uint8_t side, uint32_t now) {
    uint32_t diff = now ^ prevKeys[side];
    if (!diff) return;
    for (uint8_t i = 0; i < NUM_KEYS; i++)
        if ((diff >> i) & 1u && !((now >> i) & 1u)) keyEvent(side, i, false);
    for (uint8_t i = 0; i < NUM_KEYS; i++)
        if ((diff >> i) & 1u && ((now >> i) & 1u)) keyEvent(side, i, true);
    prevKeys[side] = now;
}
static void applyEncoderSteps(int8_t steps) {
    if (!steps) return;
    markActive();
    if (steps > 8) steps = 8;
    if (steps < -8) steps = -8;
    if (!ble.isConnected()) return;
    for (int8_t i = 0; i < abs(steps); i++) ble.write(steps > 0 ? KEY_MEDIA_VOLUME_UP : KEY_MEDIA_VOLUME_DOWN);
}
static void cycleBrightness() {
    markActive();
    brightnessLevel = (brightnessLevel + 1) % BACKLIGHT_NUM_LEVELS;
    prefs.putUChar("bl", brightnessLevel);
    Serial.printf("backlight level %u\n", brightnessLevel);
}
static void leftSetup() {
    prefs.begin("split", false);
    brightnessLevel = prefs.getUChar("bl", BACKLIGHT_DEFAULT_LEVEL) % BACKLIGHT_NUM_LEVELS;
    ble.begin();
    if (!splitBegin()) Serial.println("ESP-NOW init failed");
    markActive();
}
static void leftTick() {
    matrix.scan();
    encoder.update();
    const bool connected = ble.isConnected();
    if (connected != wasConnected) {
        wasConnected = connected;
        ble.releaseAll();
        Serial.println(connected ? "BLE connected" : "BLE disconnected");
    }
    processKeys(0, matrix.state());
    applyEncoderSteps((int8_t)(encoder.counter() - lastLocalEnc));
    lastLocalEnc = encoder.counter();
    if (encoder.button() && !lastLocalBtn) cycleBrightness();
    lastLocalBtn = encoder.button();
    RemoteState rs = {0, 0, false};
    uint32_t age = 0;
    const bool remoteUp = splitGetRemote(rs, age) && age < REMOTE_TIMEOUT_MS;
    if (remoteUp != wasRemoteUp) {
        wasRemoteUp = remoteUp;
        Serial.println(remoteUp ? "right half linked" : "right half lost");
    }
    if (remoteUp) {
        if (!remoteEncInit) {  
            lastRemoteEnc = rs.encCount;
            lastRemoteBtn = rs.encButton;
            remoteEncInit = true;
        }
        processKeys(1, rs.keys);
        applyEncoderSteps((int8_t)(rs.encCount - lastRemoteEnc));
        lastRemoteEnc = rs.encCount;
        if (rs.encButton && !lastRemoteBtn) cycleBrightness();
        lastRemoteBtn = rs.encButton;
    } else {
        remoteEncInit = false;
        processKeys(1, 0);
    }
    const uint32_t now = millis();
    const uint8_t duty = (now - lastActivityMs > BACKLIGHT_IDLE_MS) ? 0 : BACKLIGHT_LEVELS[brightnessLevel];
    if (duty != currentDuty) {
        currentDuty = duty;
        backlight::setDuty(duty);
    }
    if (duty != lastSentDuty || now - lastBacklightSendMs >= BACKLIGHT_SYNC_MS) {
        splitSendBacklight(duty);
        lastSentDuty = duty;
        lastBacklightSendMs = now;
    }
}
#endif  
#ifdef SIDE_RIGHT
static uint32_t lastKeys = 0xFFFFFFFFu;  
static uint8_t lastEnc = 0;
static bool lastBtn = false;
static uint32_t lastSendMs = 0;
static uint8_t currentDuty = 0;
static void rightSetup() {
    if (!splitBegin()) Serial.println("ESP-NOW init failed");
}
static void rightTick() {
    matrix.scan();
    encoder.update();
    const uint32_t now = millis();
    const uint32_t keys = matrix.state();
    const uint8_t enc = encoder.counter();
    const bool btn = encoder.button();
    if (keys != lastKeys || enc != lastEnc || btn != lastBtn || now - lastSendMs >= HEARTBEAT_MS) {
        splitSendState(keys, enc, btn);
        lastKeys = keys;
        lastEnc = enc;
        lastBtn = btn;
        lastSendMs = now;
    }
    uint8_t duty;
    if (splitTakeBacklight(duty) && duty != currentDuty) {
        currentDuty = duty;
        backlight::setDuty(duty);
    }
}
#endif  
void setup() {
    Serial.begin(115200);
    matrix.begin();
    encoder.begin();
    backlight::begin();
#ifdef SIDE_LEFT
    Serial.println("firmware: LEFT half");
    leftSetup();
#else
    Serial.println("firmware: RIGHT half");
    rightSetup();
#endif
}
void loop() {
#ifdef SIDE_LEFT
    leftTick();
#else
    rightTick();
#endif
    delay(1);  
}
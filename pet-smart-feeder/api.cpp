#include "api.h"
#include "feeder.h"
#include <Arduino.h>
#include <time.h>

static const char DASHBOARD_HTML[] = R"HTML(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Pet Family Smart Feeder</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#0f172a;--surface:#1e293b;--surface2:#273448;--border:#334155;
  --text:#f1f5f9;--muted:#94a3b8;
  --primary:#818cf8;--primary-d:#6366f1;
  --green:#34d399;--red:#f87171;--amber:#fbbf24;--blue:#60a5fa;
}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding-bottom:2rem}

/* Navbar */
.nav{background:var(--surface);border-bottom:1px solid var(--border);padding:.85rem 1.25rem;display:flex;align-items:center;justify-content:space-between;position:sticky;top:0;z-index:10}
.brand{display:flex;align-items:center;gap:.5rem;font-weight:700;font-size:1.05rem;letter-spacing:-.01em}
.brand-paw{font-size:1.3rem}
.nav-right{display:flex;align-items:center;gap:.6rem}
.chip{display:inline-flex;align-items:center;gap:.3rem;background:var(--surface2);border:1px solid var(--border);border-radius:6px;padding:.2rem .55rem;font-size:.72rem;color:var(--muted)}
.dot{width:9px;height:9px;border-radius:50%;background:var(--green);box-shadow:0 0 7px var(--green);animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.45}}
.dot.offline{background:var(--red);box-shadow:0 0 7px var(--red)}

/* Layout */
.wrap{max-width:680px;margin:0 auto;padding:0 1rem}
.section-label{font-size:.7rem;font-weight:600;letter-spacing:.08em;text-transform:uppercase;color:var(--muted);margin:1.4rem 0 .6rem}
.grid{display:grid;gap:.7rem}
.g2{grid-template-columns:1fr 1fr}
.g3{grid-template-columns:1fr 1fr 1fr}

/* Card */
.card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:.9rem 1.1rem}
.card-label{font-size:.68rem;font-weight:600;letter-spacing:.06em;text-transform:uppercase;color:var(--muted);margin-bottom:.35rem}
.card-val{font-size:1.55rem;font-weight:700;line-height:1}
.card-sub{font-size:.78rem;color:var(--muted);margin-top:.2rem}

/* Door card */
.door-card{display:flex;align-items:center;gap:.9rem}
.door-icon{font-size:2rem;line-height:1;flex-shrink:0}
.badge{display:inline-block;padding:.18rem .55rem;border-radius:5px;font-size:.72rem;font-weight:700;letter-spacing:.05em;text-transform:uppercase}
.b-open{background:rgba(52,211,153,.15);color:var(--green)}
.b-closed{background:rgba(248,113,113,.15);color:var(--red)}
.b-busy{background:rgba(251,191,36,.15);color:var(--amber)}

/* Food bar */
.bar-bg{height:8px;background:var(--surface2);border-radius:99px;margin-top:.45rem;overflow:hidden}
.bar-fill{height:100%;border-radius:99px;transition:width .6s ease;background:linear-gradient(90deg,var(--green),var(--blue))}
.bar-fill.low{background:linear-gradient(90deg,var(--red),var(--amber))}
.bar-row{display:flex;justify-content:space-between;align-items:center}

/* Countdown */
.countdown{font-size:1.75rem;font-weight:700;font-variant-numeric:tabular-nums;color:var(--primary)}

/* Buttons */
.btn{width:100%;padding:.85rem;border:none;border-radius:11px;font-size:.95rem;font-weight:600;cursor:pointer;transition:all .15s;display:flex;align-items:center;justify-content:center;gap:.45rem;margin-top:.7rem}
.btn-primary{background:var(--primary-d);color:#fff}
.btn-primary:hover{background:var(--primary);transform:translateY(-1px)}
.btn-primary:active{transform:translateY(0)}
.btn-primary:disabled{opacity:.45;cursor:not-allowed;transform:none}
.btn-ghost{background:var(--surface2);color:var(--muted);border:1px solid var(--border)}
.btn-ghost:hover{color:var(--text);border-color:var(--muted)}

/* Schedule toggle */
.mode-row{display:flex;gap:.5rem;margin-bottom:.9rem}
.btn-mode{flex:1;padding:.5rem;border:1px solid var(--border);border-radius:8px;background:var(--surface2);color:var(--muted);font-size:.82rem;font-weight:600;cursor:pointer;transition:all .15s}
.btn-mode.active{background:var(--primary-d);color:#fff;border-color:var(--primary-d)}

/* Schedule inputs */
.sched-row{display:flex;gap:.5rem;align-items:center;margin-top:.4rem;flex-wrap:wrap}
.sched-inp{background:var(--surface2);border:1px solid var(--border);border-radius:8px;color:var(--text);padding:.4rem .6rem;font-size:.95rem;width:4.8rem;text-align:center}
input[type=time].sched-inp{width:auto;font-size:1.15rem;padding:.4rem .8rem}
.sched-inp:focus{outline:none;border-color:var(--primary)}

/* Log */
.log-wrap{background:var(--surface);border:1px solid var(--border);border-radius:12px;overflow:hidden}
.log-row{display:flex;align-items:center;gap:.85rem;padding:.6rem 1.1rem;border-bottom:1px solid var(--border);font-size:.82rem}
.log-row:last-child{border-bottom:none}
.log-dot{width:6px;height:6px;border-radius:50%;background:var(--primary);flex-shrink:0}
.log-time{color:var(--muted);font-variant-numeric:tabular-nums;font-size:.75rem;white-space:nowrap}
.log-msg{color:var(--text)}

/* Footer */
.footer{text-align:center;margin-top:1rem;font-size:.7rem;color:var(--muted)}

@media(max-width:460px){.g3{grid-template-columns:1fr 1fr}}
</style>
</head>
<body>

<nav class="nav">
  <div class="brand">
    <span class="brand-paw">&#x1F43E;</span>
    Pet Family Smart Feeder
  </div>
  <div class="nav-right">
    <span class="chip" id="uptime">&#x23F1; --:--:--</span>
    <div class="dot" id="dot"></div>
  </div>
</nav>

<div class="wrap">

  <p class="section-label">Status da Porta</p>
  <div class="card door-card">
    <div class="door-icon" id="door-icon">&#x1F512;</div>
    <div>
      <div class="card-label">Porta</div>
      <span class="badge b-closed" id="door-badge">FECHADA</span>
      <div class="card-sub" id="door-sub">Sistema pronto</div>
    </div>
  </div>

  <p class="section-label">Ambiente &amp; Sensores</p>
  <div class="grid g3">
    <div class="card">
      <div class="card-label">Temperatura</div>
      <div class="card-val" id="temp">--</div>
      <div class="card-sub">&#xB0;C</div>
    </div>
    <div class="card">
      <div class="card-label">Umidade</div>
      <div class="card-val" id="hum">--</div>
      <div class="card-sub">%</div>
    </div>
    <div class="card">
      <div class="card-label">Alimentacoes</div>
      <div class="card-val" id="feeds">0</div>
      <div class="card-sub">hoje</div>
    </div>
  </div>

  <div class="card" style="margin-top:.7rem">
    <div class="bar-row">
      <div class="card-label" style="margin-bottom:0">Nivel de Racao</div>
      <span class="card-sub" id="food-pct">100%</span>
    </div>
    <div class="bar-bg"><div class="bar-fill" id="food-bar" style="width:100%"></div></div>
    <div class="card-sub" style="margin-top:.35rem" id="food-status">Reservatorio cheio</div>
  </div>

  <p class="section-label">Automacao</p>
  <div class="grid g2">
    <div class="card">
      <div class="card-label">Proxima Alimentacao</div>
      <div class="countdown" id="next-feed">--:--</div>
      <div class="card-sub" id="auto-label">Automatico ativo</div>
    </div>
    <div class="card">
      <div class="card-label">Ultima Alimentacao</div>
      <div class="card-val" style="font-size:1.2rem" id="last-feed">--:--:--</div>
      <div class="card-sub" id="feed-sub">Sem registros</div>
    </div>
  </div>

  <p class="section-label">Controle Manual</p>
  <button class="btn btn-primary" id="btn-feed" onclick="cmdFeed()">&#x1F37D; Alimentar Agora</button>
  <button class="btn btn-ghost" id="btn-refill" onclick="cmdRefill()" style="margin-top:.5rem">&#x1F504; Reabastecer Racao</button>

  <p class="section-label">Agendamento</p>
  <div class="card">
    <div class="mode-row">
      <button class="btn-mode active" id="tab-interval" onclick="setSchedMode('interval')">&#x23F1; Intervalo</button>
      <button class="btn-mode" id="tab-time" onclick="setSchedMode('time')">&#x1F551; Horario Fixo</button>
    </div>

    <div id="pnl-interval">
      <div class="card-label">Intervalo entre alimentacoes</div>
      <div class="sched-row">
        <input id="inp-h" type="number" min="0" max="23" value="0" class="sched-inp">
        <span class="card-sub">h</span>
        <input id="inp-m" type="number" min="0" max="59" value="20" class="sched-inp">
        <span class="card-sub">min</span>
      </div>
      <div class="card-sub" style="margin-top:.4rem">Minimo: 1 minuto</div>
    </div>

    <div id="pnl-time" style="display:none">
      <div class="card-label">Horario diario (Brasilia UTC-3)</div>
      <div class="sched-row">
        <input id="inp-time" type="time" value="08:00" class="sched-inp">
      </div>
      <div class="card-sub" style="margin-top:.4rem" id="ntp-status">Aguardando sincronizacao NTP...</div>
    </div>

    <button class="btn btn-primary" onclick="saveSchedule()" style="margin-top:.8rem">&#x1F4BE; Salvar Agendamento</button>
  </div>

  <p class="section-label">Log de Eventos</p>
  <div class="log-wrap" id="log"></div>

  <p class="footer">Atualizado: <span id="updated">--</span></p>
</div>

<script>
const STATES={
  CLOSED:  {icon:"&#x1F512;",cls:"b-closed",label:"FECHADA",  sub:"Sistema pronto"},
  OPENING: {icon:"&#x1F513;",cls:"b-busy",  label:"ABRINDO",  sub:"Aguarde..."},
  OPEN:    {icon:"&#x1F513;",cls:"b-open",  label:"ABERTA",   sub:"Distribuindo racao"},
  ALARMING:{icon:"&#x26A0;", cls:"b-busy",  label:"FECHANDO", sub:"Encerrando ciclo"},
  CLOSING: {icon:"&#x1F512;",cls:"b-busy",  label:"FECHANDO", sub:"Aguarde..."},
};

function fmt2(n){return String(n).padStart(2,"0")}
function fmtUptime(s){return`${fmt2(Math.floor(s/3600))}:${fmt2(Math.floor(s/60)%60)}:${fmt2(s%60)}`}
function fmtMs(ms){
  if(ms<=0)return"00:00";
  const s=Math.floor(ms/1000);
  return`${fmt2(Math.floor(s/60))}:${fmt2(s%60)}`;
}

function setDoor(s){
  const d=STATES[s]||STATES.CLOSED;
  document.getElementById("door-icon").innerHTML=d.icon;
  const b=document.getElementById("door-badge");
  b.className="badge "+d.cls; b.textContent=d.label;
  document.getElementById("door-sub").textContent=d.sub;
}

function setFood(pct){
  const bar=document.getElementById("food-bar");
  document.getElementById("food-pct").textContent=pct+"%";
  bar.style.width=pct+"%";
  const st=document.getElementById("food-status");
  if(pct<=0){bar.className="bar-fill low";st.textContent="Sem racao - reabastecer!";st.style.color="var(--red)"}
  else if(pct<=25){bar.className="bar-fill low";st.textContent="Nivel baixo - reabastecer";st.style.color="var(--red)"}
  else if(pct<=50){bar.className="bar-fill";st.textContent="Nivel moderado";st.style.color="var(--amber)"}
  else{bar.className="bar-fill";st.textContent="Reservatorio em bom nivel";st.style.color="var(--muted)"}
}

function setLog(logs){
  if(!logs||!logs.length)return;
  document.getElementById("log").innerHTML=logs.map(l=>
    `<div class="log-row"><div class="log-dot"></div><span class="log-time">${l.time}</span><span class="log-msg">${l.msg}</span></div>`
  ).join("");
}

let busy=false;
let schedInited=false;
let schedMode='interval';

async function post(url){
  try{await fetch(url,{method:"POST"})}catch(e){}
}

async function cmdFeed(){
  if(busy)return; busy=true;
  const b=document.getElementById("btn-feed");
  b.disabled=true; b.textContent="Processando...";
  await post("/api/feed");
  setTimeout(()=>{busy=false;b.disabled=false;b.innerHTML="&#x1F37D; Alimentar Agora"},3000);
}

async function cmdRefill(){
  await post("/api/refill");
}

function setSchedMode(m){
  schedMode=m;
  document.getElementById("pnl-interval").style.display=m==="interval"?"block":"none";
  document.getElementById("pnl-time").style.display=m==="time"?"block":"none";
  document.getElementById("tab-interval").classList.toggle("active",m==="interval");
  document.getElementById("tab-time").classList.toggle("active",m==="time");
}

async function saveSchedule(){
  if(schedMode==="interval"){
    const h=parseInt(document.getElementById("inp-h").value)||0;
    const m=parseInt(document.getElementById("inp-m").value)||0;
    const total=h*60+m;
    if(total<1){alert("Minimo 1 minuto");return;}
    await post("/api/schedule?mode=interval&minutes="+total);
  }else{
    const t=document.getElementById("inp-time").value;
    if(!t){alert("Informe um horario");return;}
    const parts=t.split(":");
    const h=parseInt(parts[0]);
    const m=parseInt(parts[1]);
    await post("/api/schedule?mode=time&hour="+h+"&min="+m);
  }
}

async function poll(){
  try{
    const r=await fetch("/api/status");
    if(!r.ok)throw 0;
    const d=await r.json();

    document.getElementById("dot").className="dot";
    setDoor(d.doorState);
    document.getElementById("temp").textContent=d.temperature>0?d.temperature.toFixed(1):"--";
    document.getElementById("hum").textContent=d.humidity>0?d.humidity.toFixed(0):"--";
    document.getElementById("feeds").textContent=d.feedCount;
    setFood(d.foodLevel);

    if(d.autoFeedEnabled){
      if(d.nextFeedIn<0){
        document.getElementById("next-feed").textContent="--:--";
      }else{
        document.getElementById("next-feed").textContent=fmtMs(d.nextFeedIn);
      }
      document.getElementById("auto-label").textContent=d.useSchedule
        ?"Agendado: "+fmt2(d.schedHour)+":"+fmt2(d.schedMin)
        :"Automatico ativo";
    }else{
      document.getElementById("next-feed").textContent="--:--";
      document.getElementById("auto-label").textContent="Modo manual";
    }

    document.getElementById("last-feed").textContent=d.lastFeedTime;
    document.getElementById("feed-sub").textContent=d.feedCount>0?`${d.feedCount} alimentacao(oes)`:"Sem registros";
    document.getElementById("uptime").innerHTML="&#x23F1; "+fmtUptime(d.uptime);
    setLog(d.logs);
    document.getElementById("updated").textContent=new Date().toLocaleTimeString("pt-BR");

    if(d.currentTime){
      document.getElementById("ntp-status").textContent="Hora atual: "+d.currentTime;
    }

    if(!schedInited && d.feedIntervalMs!==undefined){
      schedInited=true;
      if(d.useSchedule){
        setSchedMode("time");
        document.getElementById("inp-time").value=fmt2(d.schedHour)+":"+fmt2(d.schedMin);
      }else{
        setSchedMode("interval");
        const totalMin=Math.round(d.feedIntervalMs/60000);
        document.getElementById("inp-h").value=Math.floor(totalMin/60);
        document.getElementById("inp-m").value=totalMin%60;
      }
    }
  }catch(e){
    document.getElementById("dot").className="dot offline";
  }
}

poll();
setInterval(poll,2000);
</script>
</body>
</html>
)HTML";

static WebServer* srv = nullptr;

static const char* _door_state_str() {
    switch (state.doorState) {
        case DOOR_CLOSED:   return "CLOSED";
        case DOOR_OPENING:  return "OPENING";
        case DOOR_OPEN:     return "OPEN";
        case DOOR_ALARMING: return "ALARMING";
        case DOOR_CLOSING:  return "CLOSING";
        default:            return "CLOSED";
    }
}

static void _on_root() {
    srv->send(200, "text/html", DASHBOARD_HTML);
}

static void _on_status() {
    unsigned long now    = millis();
    unsigned long uptime = (now - state.bootMs) / 1000;

    long nextFeedIn = 0;
    if (state.doorState == DOOR_CLOSED && state.autoFeedEnabled) {
        if (state.useSchedule) {
            struct tm t;
            if (getLocalTime(&t)) {
                int nowMin = t.tm_hour * 60 + t.tm_min;
                int schMin = state.schedHour * 60 + state.schedMin;
                int diffMin = schMin - nowMin;
                if (diffMin <= 0) diffMin += 24 * 60;
                nextFeedIn = (long)diffMin * 60000L - (long)t.tm_sec * 1000L;
            } else {
                nextFeedIn = -1;
            }
        } else {
            long elapsed = (long)(now - state.lastAutoFeedAt);
            nextFeedIn = max(0L, (long)state.feedIntervalMs - elapsed);
        }
    }

    // Hora NTP atual
    char currentTime[10] = "";
    struct tm t;
    if (getLocalTime(&t)) {
        snprintf(currentTime, sizeof(currentTime), "%02d:%02d:%02d",
                 t.tm_hour, t.tm_min, t.tm_sec);
    }

    String j = "{";
    j += "\"doorState\":\"";      j += _door_state_str();          j += "\",";
    j += "\"doorOpen\":";         j += feeder_is_open() ? "true" : "false"; j += ",";
    j += "\"autoFeedEnabled\":";  j += state.autoFeedEnabled ? "true" : "false"; j += ",";
    j += "\"useSchedule\":";      j += state.useSchedule ? "true" : "false"; j += ",";
    j += "\"feedIntervalMs\":";   j += state.feedIntervalMs;        j += ",";
    j += "\"schedHour\":";        j += state.schedHour;             j += ",";
    j += "\"schedMin\":";         j += state.schedMin;              j += ",";
    j += "\"nextFeedIn\":";       j += nextFeedIn;                  j += ",";
    j += "\"lastFeedTime\":\"";   j += state.lastFeedTime;          j += "\",";
    j += "\"feedCount\":";        j += state.feedCount;             j += ",";
    j += "\"foodLevel\":";        j += state.foodLevel;             j += ",";
    j += "\"temperature\":";      j += String(state.temperature, 1); j += ",";
    j += "\"humidity\":";         j += String(state.humidity,    1); j += ",";
    j += "\"uptime\":";           j += uptime;                      j += ",";
    if (currentTime[0]) {
        j += "\"currentTime\":\""; j += currentTime; j += "\",";
    } else {
        j += "\"currentTime\":null,";
    }
    j += "\"logs\":[";
    for (int i = 0; i < state.logCount; i++) {
        if (i) j += ",";
        j += "{\"time\":\""; j += state.logs[i].time; j += "\",";
        j += "\"msg\":\"";   j += state.logs[i].msg;  j += "\"}";
    }
    j += "]}";

    srv->send(200, "application/json", j);
}

static void _on_feed() {
    if (srv->method() != HTTP_POST) {
        srv->send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
        return;
    }
    if (state.doorState != DOOR_CLOSED) {
        srv->send(409, "application/json", "{\"ok\":false,\"error\":\"Feeder busy\"}");
        return;
    }
    feeder_trigger_open("Alimentacao manual via dashboard");
    srv->send(200, "application/json", "{\"ok\":true}");
}

static void _on_close() {
    if (srv->method() != HTTP_POST) {
        srv->send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
        return;
    }
    feeder_trigger_close("Fechamento manual via dashboard");
    srv->send(200, "application/json", "{\"ok\":true}");
}

static void _on_refill() {
    if (srv->method() != HTTP_POST) {
        srv->send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
        return;
    }
    state.foodLevel = FOOD_LEVEL_INITIAL;

    int top = min(state.logCount, LOG_MAX - 1);
    for (int i = top; i > 0; i--) state.logs[i] = state.logs[i - 1];
    unsigned long s = millis() / 1000;
    snprintf(state.logs[0].time, LOG_TIME_LEN, "%02lu:%02lu:%02lu",
             (s / 3600) % 24, (s / 60) % 60, s % 60);
    strncpy(state.logs[0].msg, "Racao reabastecida (100%)", LOG_MSG_LEN - 1);
    if (state.logCount < LOG_MAX) state.logCount++;

    srv->send(200, "application/json", "{\"ok\":true,\"foodLevel\":100}");
}

static void _on_schedule() {
    if (srv->method() != HTTP_POST) {
        srv->send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
        return;
    }

    String mode = srv->arg("mode");

    if (mode == "interval") {
        int minutes = srv->arg("minutes").toInt();
        if (minutes < 1 || minutes > 1440) {
            srv->send(400, "application/json", "{\"ok\":false,\"error\":\"minutes must be 1-1440\"}");
            return;
        }
        state.useSchedule    = false;
        state.feedIntervalMs = (unsigned long)minutes * 60000UL;
        state.lastAutoFeedAt = millis();

        char msg[LOG_MSG_LEN];
        snprintf(msg, sizeof(msg), "Intervalo configurado: %d min", minutes);
        int top = min(state.logCount, LOG_MAX - 1);
        for (int i = top; i > 0; i--) state.logs[i] = state.logs[i - 1];
        unsigned long sec = millis() / 1000;
        snprintf(state.logs[0].time, LOG_TIME_LEN, "%02lu:%02lu:%02lu",
                 (sec / 3600) % 24, (sec / 60) % 60, sec % 60);
        strncpy(state.logs[0].msg, msg, LOG_MSG_LEN - 1);
        if (state.logCount < LOG_MAX) state.logCount++;

        srv->send(200, "application/json", "{\"ok\":true}");

    } else if (mode == "time") {
        int hour    = srv->arg("hour").toInt();
        int minArg  = srv->arg("min").toInt();
        if (hour < 0 || hour > 23 || minArg < 0 || minArg > 59) {
            srv->send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid time\"}");
            return;
        }
        state.useSchedule          = true;
        state.schedHour            = hour;
        state.schedMin             = minArg;
        state.schedFiredThisMinute = false;

        char msg[LOG_MSG_LEN];
        snprintf(msg, sizeof(msg), "Horario configurado: %02d:%02d", hour, minArg);
        int top = min(state.logCount, LOG_MAX - 1);
        for (int i = top; i > 0; i--) state.logs[i] = state.logs[i - 1];
        unsigned long sec = millis() / 1000;
        snprintf(state.logs[0].time, LOG_TIME_LEN, "%02lu:%02lu:%02lu",
                 (sec / 3600) % 24, (sec / 60) % 60, sec % 60);
        strncpy(state.logs[0].msg, msg, LOG_MSG_LEN - 1);
        if (state.logCount < LOG_MAX) state.logCount++;

        srv->send(200, "application/json", "{\"ok\":true}");

    } else {
        srv->send(400, "application/json", "{\"ok\":false,\"error\":\"mode must be interval or time\"}");
    }
}

static void _on_not_found() {
    srv->send(404, "application/json", "{\"ok\":false,\"error\":\"Not found\"}");
}

void api_begin(WebServer& server) {
    srv = &server;

    server.on("/",              HTTP_GET,  _on_root);
    server.on("/api/status",    HTTP_GET,  _on_status);
    server.on("/api/feed",      HTTP_POST, _on_feed);
    server.on("/api/close",     HTTP_POST, _on_close);
    server.on("/api/refill",    HTTP_POST, _on_refill);
    server.on("/api/schedule",  HTTP_POST, _on_schedule);
    server.onNotFound(_on_not_found);

    server.begin();
    Serial.println("WebServer iniciado na porta 80.");
}

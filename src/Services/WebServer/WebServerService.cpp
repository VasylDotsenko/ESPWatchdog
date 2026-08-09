#include "WebServerService.h"

#include "Core/Application.h"
#include "Serializers/JsonStatusSerializer.h"
#include "Services/Logger/Logger.h"

WebServerService WebServer;

namespace
{
    const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="uk">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP Watchdog</title>
<style>
:root{color-scheme:dark;--bg:#0b1020;--card:#151b2e;--text:#e8edf7;--muted:#8e9bb4;--ok:#2ee59d;--warn:#ffd166;--bad:#ff5d73;--line:#26314d}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:14px/1.45 system-ui,-apple-system,Segoe UI,sans-serif}
header{padding:22px 18px;border-bottom:1px solid var(--line);background:#0f1628;position:sticky;top:0}
h1{margin:0;font-size:22px}.sub{color:var(--muted);margin-top:4px}
main{padding:16px;display:grid;gap:14px;grid-template-columns:repeat(auto-fit,minmax(250px,1fr))}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px;box-shadow:0 10px 28px #0004}
.card h2{font-size:15px;margin:0 0 10px;color:#fff}.row{display:flex;justify-content:space-between;gap:12px;border-top:1px solid var(--line);padding:7px 0}
.row:first-of-type{border-top:0}.k{color:var(--muted)}.v{text-align:right;word-break:break-word}.ok{color:var(--ok)}.warn{color:var(--warn)}.bad{color:var(--bad)}
footer{padding:0 18px 18px;color:var(--muted)}code{color:#c6d3ff}
</style>
</head>
<body>
<header>
<h1>ESP Watchdog</h1>
<div class="sub">Live status · <span id="updated">loading...</span></div>
</header>
<main id="app"></main>
<footer>API: <code>/api/status</code> · Health: <code>/health</code></footer>
<script>
const app=document.getElementById('app'),updated=document.getElementById('updated');
const ip=a=>a||'0.0.0.0';
const cls=b=>b?'ok':'bad';
const ms=v=>v?`${v} ms`:'0 ms';
function row(k,v,c=''){return `<div class="row"><span class="k">${k}</span><span class="v ${c}">${v}</span></div>`}
function card(t,rows){return `<section class="card"><h2>${t}</h2>${rows.join('')}</section>`}
function render(s){
 const sys=s.system||{}, fw=sys.firmware||{}, up=sys.uptime||{}, mem=sys.memory||{}, cpu=sys.cpu||{};
 const net=s.network||{}, ns=net.summary||{}, nc=net.configuration||{}, na=net.address||{}, sig=net.signal||{};
 const h=s.health||{}, hs=h.summary||{}, hst=h.statistics||{};
 const w=s.watchdog||{}, ws=w.summary||{}, wc=w.configuration||{}, wst=w.statistics||{};
 const p=s.power||{}, ps=p.summary||{}, pst=p.statistics||{}, ph=p.history||{};
 app.innerHTML=[
  card('System',[row('Version',fw.version||'-'),row('Build',`${fw.buildDate||''} ${fw.buildTime||''}`),row('Uptime',`${up.days||0}d ${up.hours||0}h ${up.minutes||0}m`),row('Heap',mem.freeHeap||0),row('CPU',`${cpu.frequencyMHz||0} MHz`)]),
  card('Network',[row('State',ns.stateText||'-',cls(ns.connected)),row('IP',ip(na.ip)),row('SSID',nc.ssid||'-'),row('RSSI',`${sig.rssi||0} dBm`),row('Quality',`${sig.quality||0}%`)]),
  card('Health',[row('Available',hs.available?'online':'offline',cls(hs.available)),row('Last status',hs.lastStatusText||'-'),row('RTT',ms(hs.responseTime)),row('Sent',hst.sent||0),row('Lost',hst.lost||0),row('Fails',hst.consecutiveFails||0)]),
  card('Watchdog',[row('State',ws.stateText||'-',ws.lockedOut?'bad':ws.cooldown?'warn':'ok'),row('Enabled',ws.enabled?'yes':'no',cls(ws.enabled)),row('Failures',`${ws.consecutiveFailures||0}/${wc.failureThreshold||0}`),row('Restarts',wst.restartCount||0),row('Power off',ms(wc.powerOffTime))]),
  card('Power',[row('State',ps.stateText||'-',ps.available?'ok':'warn'),row('Controller',ps.available?'available':'unavailable',cls(ps.available)),row('Restarting',ps.restartInProgress?'yes':'no',ps.restartInProgress?'warn':''),row('Restarts',pst.restartCount||0),row('Errors',pst.errorCount||0),row('History',`${ph.succeeded||0} ok / ${ph.failed||0} failed`)])
 ].join('');
 updated.textContent=new Date().toLocaleTimeString();
}
async function load(){try{const r=await fetch('/api/status',{cache:'no-store'});render(await r.json())}catch(e){updated.textContent='offline';app.innerHTML=card('Error',[row('Status','unable to load','bad')])}}
load();setInterval(load,2000);
</script>
</body>
</html>
)HTML";
}

bool WebServerService::begin()
{
    configureRoutes();

    m_server.begin();

    Log.info(
        "WebServer: started, port=%u",
        DEFAULT_PORT);

    return true;
}

void WebServerService::loop()
{
    m_server.handleClient();
}

void WebServerService::configureRoutes()
{
    m_server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        });

    m_server.on(
        "/api/status",
        HTTP_GET,
        [this]()
        {
            handleApiStatus();
        });

    m_server.on(
        "/health",
        HTTP_GET,
        [this]()
        {
            handleHealth();
        });

    m_server.onNotFound(
        [this]()
        {
            handleNotFound();
        });
}

void WebServerService::handleRoot()
{
    m_server.sendHeader(
        "Cache-Control",
        "no-store");

    m_server.send_P(
        200,
        "text/html",
        INDEX_HTML);
}

void WebServerService::handleApiStatus()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serialize(
            App.status(),
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(
            500,
            "{\"error\":\"status_serialization_failed\"}");

        return;
    }

    sendJson(
        200,
        m_jsonBuffer);
}

void WebServerService::handleHealth()
{
    sendJson(
        200,
        "{\"status\":\"ok\"}");
}

void WebServerService::handleNotFound()
{
    sendJson(
        404,
        "{\"error\":\"not_found\"}");
}

void WebServerService::sendJson(
    int statusCode,
    const char* json)
{
    m_server.sendHeader(
        "Access-Control-Allow-Origin",
        "*");

    m_server.sendHeader(
        "Cache-Control",
        "no-store");

    m_server.send(
        statusCode,
        "application/json",
        json != nullptr ? json : "{}");
}

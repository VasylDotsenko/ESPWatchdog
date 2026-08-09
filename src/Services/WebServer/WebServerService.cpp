#include "WebServerService.h"

#include <ArduinoJson.h>
#include <cstring>

#include "Core/Application.h"
#include "Services/Config/Config.h"
#include "Serializers/JsonStatusSerializer.h"
#include "Services/Logger/Logger.h"
#include "Services/Power/PowerService.h"

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
button{border:0;border-radius:10px;padding:10px 12px;background:#33415f;color:#fff;font-weight:700;cursor:pointer}button:hover{filter:brightness(1.1)}button:disabled{opacity:.45;cursor:not-allowed}.btns{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}.danger{background:#8f2635}.warnBtn{background:#8b6822}.okBtn{background:#16724e}.log{max-height:180px;overflow:auto;background:#0a0f1d;border:1px solid var(--line);border-radius:10px;padding:10px;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:12px}.log div{padding:2px 0;color:#cdd7ee}.log .warn{color:var(--warn)}.log .bad{color:var(--bad)}.log .ok{color:var(--ok)}
input{width:100%;margin-top:4px;border:1px solid var(--line);border-radius:9px;background:#0a0f1d;color:var(--text);padding:8px}.field{margin:8px 0}.field label{display:block;color:var(--muted);font-size:12px}.wide{grid-column:1/-1}.formGrid{display:grid;gap:10px;grid-template-columns:repeat(auto-fit,minmax(190px,1fr))}
footer{padding:0 18px 18px;color:var(--muted)}code{color:#c6d3ff}a{color:#9db7ff;text-decoration:none}a:hover{text-decoration:underline}.links{display:flex;flex-wrap:wrap;gap:10px;margin-top:8px}
</style>
</head>
<body>
<header>
<h1>ESP Watchdog</h1>
<div class="sub">Live status · <span id="updated">loading...</span></div>
<div class="links">
<a href="/">dashboard</a>
<a href="/config/device">device</a>
<a href="/config/wifi">wifi</a>
<a href="/config/watchdog">watchdog</a>
<a href="/config/relay">relay</a>
<a href="/config/tuya">tuya</a>
<a href="/config/security">security</a>
<a href="/logs">logs</a>
<a href="#" onclick="setApiToken();return false">api token</a>
</div>
</header>
<main id="app"></main>
<footer>
<div>API endpoints</div>
<div class="links">
<a href="/api">index</a>
<a href="/api/status">status</a>
<a href="/api/system">system</a>
<a href="/api/network">network</a>
<a href="/api/health">health</a>
<a href="/api/watchdog">watchdog</a>
<a href="/api/power">power</a>
<a href="/api/config">config</a>
<a href="/api/logs">logs</a>
</div>
</footer>
<script>
const app=document.getElementById('app'),updated=document.getElementById('updated');
const commandLog=[];
const ip=a=>a||'0.0.0.0';
const cls=b=>b?'ok':'bad';
const ms=v=>v?`${v} ms`:'0 ms';
const esc=v=>String(v??'').replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
function row(k,v,c=''){return `<div class="row"><span class="k">${k}</span><span class="v ${c}">${v}</span></div>`}
function card(t,rows){return `<section class="card"><h2>${t}</h2>${rows.join('')}</section>`}
function logLine(t,c=''){return `<div class="${c}">${t}</div>`}
function addLog(t,c=''){commandLog.unshift(`[${new Date().toLocaleTimeString()}] ${t}`);if(commandLog.length>20)commandLog.pop();load()}
async function powerCommand(name,path,confirmText=''){
 if(confirmText&&!confirm(confirmText))return;
 addLog(`${name}: sending`,'warn');
 try{
  const r=await fetch(path,{method:'POST',cache:'no-store',headers:authHeaders()});
  const txt=await r.text();
  addLog(`${name}: HTTP ${r.status} ${txt}`,r.ok?'ok':'bad');
 }catch(e){addLog(`${name}: ${e.message||'failed'}`,'bad')}
}
function apiToken(){return localStorage.getItem('espw_api_token')||''}
function authHeaders(extra={}){const t=apiToken();if(t)extra['Authorization']='Bearer '+t;return extra}
function setApiToken(){const v=prompt('API token for this browser',apiToken());if(v!==null){localStorage.setItem('espw_api_token',v);addLog('API token saved in browser','ok')}}
function controls(ps,wc){
 const disabled=!ps.available||ps.restartInProgress?'disabled':'';
 const restartText=`Restart power for ${(wc.powerOffTime||10000)} ms?`;
 return `<section class="card"><h2>Power controls</h2>
  ${row('Controller',ps.available?'available':'unavailable',cls(ps.available))}
  ${row('Restarting',ps.restartInProgress?'yes':'no',ps.restartInProgress?'warn':'')}
  <div class="btns">
   <button class="okBtn" ${disabled} onclick="powerCommand('Power ON','/api/power/on')">ON</button>
   <button class="danger" ${disabled} onclick="powerCommand('Power OFF','/api/power/off','Turn power OFF?')">OFF</button>
   <button class="warnBtn" ${disabled} onclick="powerCommand('Restart','/api/power/restart','${restartText}')">RESTART</button>
  </div>
 </section>`;
}
function restartHistory(ph){
 const entries=(ph.entries||[]).slice().reverse().slice(0,8);
 if(!entries.length)return card('Restart history',[row('Entries','none')]);
 return `<section class="card"><h2>Restart history</h2><div class="log">`+
  entries.map(e=>logLine(`#${e.id} ${e.reasonText||'unknown'} → ${e.resultText||'none'} · off=${e.requestedPowerOffTime||0} ms · dur=${e.actualDuration||0} ms`,e.resultText==='success'?'ok':(e.resultText==='failed'?'bad':'warn'))).join('')+
  `</div></section>`;
}
function commandLogCard(){
 const lines=commandLog.length?commandLog.map(x=>logLine(x)).join(''):logLine('No manual commands yet');
 return `<section class="card"><h2>Command log</h2><div class="log">${lines}</div></section>`;
}
function currentPage(){
 if(location.pathname==='/logs')return 'logs';
 return '';
}
function logsCard(data){
 const entries=(data&&data.entries)||[];
 const lines=entries.length?entries.slice().reverse().map(e=>{
  const c=e.levelText==='ERROR'?'bad':(e.levelText==='WARN '?'warn':'');
  return logLine(`[${String(e.timestamp||0).padStart(10,'0')}][${e.levelText||'-'}] ${esc(e.message||'')}`,c);
 }).join(''):logLine('No runtime logs yet');
 return `<section class="card wide"><h2>Runtime logs</h2><div class="btns"><button onclick="load()">REFRESH</button></div><div class="log">${lines}</div></section>`;
}
function configCards(c){
 const wd=(c&&c.watchdog)||{}, tu=(c&&c.tuya)||{}, sec=(c&&c.security)||{};
 const proto=tu.version?`${Math.floor(tu.version/10)}.${tu.version%10}`:'-';
 return [
  card('Controlled host',[row('Host',wd.targetHost||'-'),row('Port',wd.targetPort||0),row('Ping interval',ms(wd.pingInterval)),row('Ping timeout',ms(wd.pingTimeout)),row('Fail count',wd.failCount||0)]),
  card('Tuya socket',[row('IP',tu.ip||'-'),row('Port',tu.port||0),row('Protocol',proto),row('Relay DPS',tu.relayDps||0),row('Status polling',tu.statusPollingEnabled?'enabled':'disabled'),row('Poll interval',ms(tu.statusPollingInterval||0)),row('Device ID',tu.deviceId||'-'),row('Local key',tu.localKeyMasked||'-')]),
  card('Security',[row('API auth',sec.apiAuthEnabled?'enabled':'disabled',sec.apiAuthEnabled?'warn':'ok'),row('API token',sec.apiTokenSet?'set':'not set',sec.apiTokenSet?'ok':'warn')])
 ];
}
function field(id,label,value,type='text'){return `<div class="field"><label for="${id}">${label}</label><input id="${id}" type="${type}" value="${esc(value)}"></div>`}
function secretField(id,label,placeholder){return `<div class="field"><label for="${id}">${label}</label><input id="${id}" type="password" value="" placeholder="${esc(placeholder||'leave empty to keep')}"></div>`}
function check(id,label,value){return `<div class="field"><label for="${id}">${label}</label><input id="${id}" type="checkbox" ${value?'checked':''}></div>`}
function n(id){return Number(document.getElementById(id).value||0)}
function s(id){return document.getElementById(id).value}
function b(id){return document.getElementById(id).checked}
function currentSection(){
 const p=location.pathname;
 if(p==='/config/device')return 'device';
 if(p==='/config/wifi')return 'wifi';
 if(p==='/config/watchdog')return 'watchdog';
 if(p==='/config/relay')return 'relay';
 if(p==='/config/tuya')return 'tuya';
 if(p==='/config/security')return 'security';
 return '';
}
function configNav(){
 return `<section class="card wide"><h2>Configuration</h2><div class="links">
  <a href="/config/device">Device</a>
  <a href="/config/wifi">WiFi</a>
  <a href="/config/watchdog">Watchdog</a>
  <a href="/config/relay">Relay</a>
  <a href="/config/tuya">Tuya</a>
  <a href="/config/security">Security</a>
 </div></section>`;
}
function sectionEditor(section,c){
 const d=(c&&c.device)||{}, wf=(c&&c.wifi)||{}, wd=(c&&c.watchdog)||{}, r=(c&&c.relay)||{}, tu=(c&&c.tuya)||{}, sec=(c&&c.security)||{};
 let title='Configuration', fields='';
 if(section==='device'){title='Device settings';fields=field('cfg_device_hostname','device.hostname',d.hostname||'')}
 if(section==='wifi'){title='WiFi settings';fields=field('cfg_wifi_ssid','wifi.ssid',wf.ssid||'')+secretField('cfg_wifi_password','wifi.password',wf.passwordMasked||'leave empty to keep')+field('cfg_wifi_reconnect','wifi.reconnectInterval',wf.reconnectInterval||0,'number')+field('cfg_wifi_timeout','wifi.connectTimeout',wf.connectTimeout||0,'number')}
 if(section==='watchdog'){title='Controlled host / Watchdog settings';fields=field('cfg_wd_host','watchdog.targetHost',wd.targetHost||'')+field('cfg_wd_port','watchdog.targetPort',wd.targetPort||0,'number')+field('cfg_wd_interval','watchdog.pingInterval',wd.pingInterval||0,'number')+field('cfg_wd_timeout','watchdog.pingTimeout',wd.pingTimeout||0,'number')+field('cfg_wd_fail','watchdog.failCount',wd.failCount||0,'number')+field('cfg_wd_boot','watchdog.bootDelay',wd.bootDelay||0,'number')+field('cfg_wd_off','watchdog.powerOffTime',wd.powerOffTime||0,'number')+field('cfg_wd_max','watchdog.maxRestartPerDay',wd.maxRestartPerDay||0,'number')}
 if(section==='relay'){title='Relay settings';fields=check('cfg_relay_enabled','relay.enabled',r.enabled)+field('cfg_relay_pin','relay.pin',r.pin||0,'number')+check('cfg_relay_active','relay.activeHigh',r.activeHigh)}
 if(section==='tuya'){title='Tuya socket settings';fields=field('cfg_tuya_ip','tuya.ip',tu.ip||'')+field('cfg_tuya_port','tuya.port',tu.port||0,'number')+field('cfg_tuya_device','tuya.deviceId',tu.deviceId||'')+secretField('cfg_tuya_key','tuya.localKey',tu.localKeyMasked||'leave empty to keep')+field('cfg_tuya_ver','tuya.version',tu.version||35,'number')+field('cfg_tuya_dps','tuya.relayDps',tu.relayDps||1,'number')+check('cfg_tuya_poll_enabled','tuya.statusPollingEnabled',tu.statusPollingEnabled)+field('cfg_tuya_poll_interval','tuya.statusPollingInterval',tu.statusPollingInterval||60000,'number')}
 if(section==='security'){title='Web API security';fields=check('cfg_sec_enabled','security.apiAuthEnabled',sec.apiAuthEnabled)+secretField('cfg_sec_token','security.apiToken',sec.apiTokenMasked||'leave empty to keep current token')}
 return `<section class="card wide"><h2>${title}</h2><div class="formGrid">${fields}</div><div class="btns"><button class="okBtn" onclick="saveSection('${section}')">SAVE ${section.toUpperCase()}</button><button class="warnBtn" onclick="restartEsp()">RESTART ESP</button></div></section>`;
}
function sectionBody(section){
 const body={version:3};
 if(section==='device')body.device={hostname:s('cfg_device_hostname')};
 if(section==='wifi'){body.wifi={ssid:s('cfg_wifi_ssid'),reconnectInterval:n('cfg_wifi_reconnect'),connectTimeout:n('cfg_wifi_timeout')};const p=s('cfg_wifi_password');if(p)body.wifi.password=p}
 if(section==='watchdog')body.watchdog={targetHost:s('cfg_wd_host'),targetPort:n('cfg_wd_port'),pingInterval:n('cfg_wd_interval'),pingTimeout:n('cfg_wd_timeout'),failCount:n('cfg_wd_fail'),bootDelay:n('cfg_wd_boot'),powerOffTime:n('cfg_wd_off'),maxRestartPerDay:n('cfg_wd_max')};
 if(section==='relay')body.relay={enabled:b('cfg_relay_enabled'),pin:n('cfg_relay_pin'),activeHigh:b('cfg_relay_active')};
 if(section==='tuya'){body.tuya={ip:s('cfg_tuya_ip'),port:n('cfg_tuya_port'),deviceId:s('cfg_tuya_device'),version:n('cfg_tuya_ver'),relayDps:n('cfg_tuya_dps'),statusPollingEnabled:b('cfg_tuya_poll_enabled'),statusPollingInterval:n('cfg_tuya_poll_interval')};const k=s('cfg_tuya_key');if(k)body.tuya.localKey=k}
 if(section==='security'){body.security={apiAuthEnabled:b('cfg_sec_enabled')};const t=s('cfg_sec_token');if(t)body.security.apiToken=t}
 return body;
}
async function saveSection(section){
 addLog(`Config ${section}: saving`,'warn');
 const body=sectionBody(section);
 try{const r=await fetch('/api/config',{method:'POST',headers:authHeaders({'Content-Type':'application/json'}),body:JSON.stringify(body)});const txt=await r.text();addLog(`Config: HTTP ${r.status} ${txt}`,r.ok?'ok':'bad')}catch(e){addLog(`Config: ${e.message||'failed'}`,'bad')}
}
async function restartEsp(){
 if(!confirm('Restart ESP now?'))return;
 addLog('ESP restart: sending','warn');
 try{const r=await fetch('/api/system/restart',{method:'POST',cache:'no-store',headers:authHeaders()});const txt=await r.text();addLog(`ESP restart: HTTP ${r.status} ${txt}`,r.ok?'ok':'bad')}catch(e){addLog(`ESP restart: ${e.message||'failed'}`,'bad')}
}
async function renderLogs(){
 try{const r=await fetch('/api/logs',{cache:'no-store'});app.innerHTML=logsCard(await r.json())+commandLogCard();updated.textContent=new Date().toLocaleTimeString()}catch(e){updated.textContent='offline';app.innerHTML=card('Error',[row('Logs','unable to load','bad')])}
}
function render(s,c){
 if(currentPage()==='logs'){
  renderLogs();
  return;
 }
 const section=currentSection();
 if(section){
  app.innerHTML=[
   configNav(),
   sectionEditor(section,c),
   commandLogCard()
  ].join('');
  updated.textContent=new Date().toLocaleTimeString();
  return;
 }
 const sys=s.system||{}, fw=sys.firmware||{}, up=sys.uptime||{}, mem=sys.memory||{}, cpu=sys.cpu||{};
 const net=s.network||{}, ns=net.summary||{}, nc=net.configuration||{}, na=net.address||{}, sig=net.signal||{};
 const h=s.health||{}, hs=h.summary||{}, hst=h.statistics||{};
 const w=s.watchdog||{}, ws=w.summary||{}, wc=w.configuration||{}, wst=w.statistics||{};
 const p=s.power||{}, ps=p.summary||{}, pst=p.statistics||{}, ph=p.history||{};
 app.innerHTML=[
  card('System',[row('Version',fw.version||'-'),row('Build',`${fw.buildDate||''} ${fw.buildTime||''}`),row('Uptime',`${up.days||0}d ${up.hours||0}h ${up.minutes||0}m`),row('Heap',mem.freeHeap||0),row('CPU',`${cpu.frequencyMHz||0} MHz`),'<div class="btns"><button class="warnBtn" onclick="restartEsp()">RESTART ESP</button></div>']),
  card('Network',[row('State',ns.stateText||'-',cls(ns.connected)),row('IP',ip(na.ip)),row('SSID',nc.ssid||'-'),row('RSSI',`${sig.rssi||0} dBm`),row('Quality',`${sig.quality||0}%`)]),
  card('Health',[row('Available',hs.available?'online':'offline',cls(hs.available)),row('Last status',hs.lastStatusText||'-'),row('RTT',ms(hs.responseTime)),row('Sent',hst.sent||0),row('Lost',hst.lost||0),row('Fails',hst.consecutiveFails||0)]),
  card('Watchdog',[row('State',ws.stateText||'-',ws.lockedOut?'bad':ws.cooldown?'warn':'ok'),row('Enabled',ws.enabled?'yes':'no',cls(ws.enabled)),row('Failures',`${ws.consecutiveFailures||0}/${wc.failureThreshold||0}`),row('Restarts',wst.restartCount||0),row('Power off',ms(wc.powerOffTime))]),
  card('Power',[row('State',ps.stateText||'-',ps.available?'ok':'warn'),row('Controller',ps.available?'available':'unavailable',cls(ps.available)),row('Restarting',ps.restartInProgress?'yes':'no',ps.restartInProgress?'warn':''),row('Restarts',pst.restartCount||0),row('Errors',pst.errorCount||0),row('History',`${ph.succeeded||0} ok / ${ph.failed||0} failed`)]),
  ...configCards(c),
  controls(ps,wc),
  restartHistory(ph),
  commandLogCard()
 ].join('');
 updated.textContent=new Date().toLocaleTimeString();
}
async function load(){try{if(currentPage()==='logs'){await renderLogs();return}if(document.activeElement&&document.activeElement.tagName==='INPUT')return;const section=currentSection();if(section){const cr=await fetch('/api/config',{cache:'no-store'});render({},await cr.json());return}const [sr,cr]=await Promise.all([fetch('/api/status',{cache:'no-store'}),fetch('/api/config',{cache:'no-store'})]);render(await sr.json(),await cr.json())}catch(e){updated.textContent='offline';app.innerHTML=card('Error',[row('Status','unable to load','bad')])}}
load();setInterval(load,2000);
</script>
</body>
</html>
)HTML";

    void sendJsonEscaped(
        ESP8266WebServer& server,
        const char* value)
    {
        if (value == nullptr)
        {
            return;
        }

        char out[7] {};

        for (const char* p = value; *p != '\0'; ++p)
        {
            const unsigned char ch =
                static_cast<unsigned char>(*p);

            switch (ch)
            {
                case '\"':
                    server.sendContent("\\\"");
                    break;

                case '\\':
                    server.sendContent("\\\\");
                    break;

                case '\b':
                    server.sendContent("\\b");
                    break;

                case '\f':
                    server.sendContent("\\f");
                    break;

                case '\n':
                    server.sendContent("\\n");
                    break;

                case '\r':
                    server.sendContent("\\r");
                    break;

                case '\t':
                    server.sendContent("\\t");
                    break;

                default:
                    if (ch < 0x20)
                    {
                        snprintf(
                            out,
                            sizeof(out),
                            "\\u%04x",
                            ch);

                        server.sendContent(out);
                    }
                    else
                    {
                        out[0] =
                            static_cast<char>(ch);
                        out[1] =
                            '\0';

                        server.sendContent(out);
                    }

                    break;
            }
        }
    }
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

    if (m_restartRequested &&
        static_cast<int32_t>(millis() - m_restartAt) >= 0)
    {
        Log.warning("WebServer: ESP restart requested");

        ESP.restart();
    }
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
        "/api",
        HTTP_GET,
        [this]()
        {
            handleApiIndex();
        });

    m_server.on(
        "/api/system",
        HTTP_GET,
        [this]()
        {
            handleApiSystem();
        });

    m_server.on(
        "/api/system/restart",
        HTTP_POST,
        [this]()
        {
            handleApiSystemRestart();
        });

    m_server.on(
        "/api/system/restart",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/network",
        HTTP_GET,
        [this]()
        {
            handleApiNetwork();
        });

    m_server.on(
        "/api/health",
        HTTP_GET,
        [this]()
        {
            handleApiHealth();
        });

    m_server.on(
        "/api/watchdog",
        HTTP_GET,
        [this]()
        {
            handleApiWatchdog();
        });

    m_server.on(
        "/api/power",
        HTTP_GET,
        [this]()
        {
            handleApiPower();
        });

    m_server.on(
        "/api/config",
        HTTP_GET,
        [this]()
        {
            handleApiConfig();
        });

    m_server.on(
        "/api/logs",
        HTTP_GET,
        [this]()
        {
            handleApiLogs();
        });

    m_server.on(
        "/api/config",
        HTTP_POST,
        [this]()
        {
            handleApiConfigUpdate();
        });

    m_server.on(
        "/api/config",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/config/device",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/wifi",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/watchdog",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/relay",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/tuya",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/security",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/logs",
        HTTP_GET,
        [this]()
        {
            handleLogsPage();
        });

    m_server.on(
        "/api/power/on",
        HTTP_POST,
        [this]()
        {
            handleApiPowerOn();
        });

    m_server.on(
        "/api/power/off",
        HTTP_POST,
        [this]()
        {
            handleApiPowerOff();
        });

    m_server.on(
        "/api/power/restart",
        HTTP_POST,
        [this]()
        {
            handleApiPowerRestart();
        });

    m_server.on(
        "/api/power/on",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/power/off",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/power/restart",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
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

void WebServerService::handleApiIndex()
{
    sendJson(
        200,
        "{"
        "\"name\":\"ESP Watchdog API\","
        "\"version\":1,"
        "\"endpoints\":["
        "{\"method\":\"GET\",\"path\":\"/\",\"description\":\"dashboard\"},"
        "{\"method\":\"GET\",\"path\":\"/api\",\"description\":\"api index\"},"
        "{\"method\":\"GET\",\"path\":\"/api/status\",\"description\":\"aggregate status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/system\",\"description\":\"system status\"},"
        "{\"method\":\"POST\",\"path\":\"/api/system/restart\",\"description\":\"restart ESP\"},"
        "{\"method\":\"GET\",\"path\":\"/api/network\",\"description\":\"network status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/health\",\"description\":\"health status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/watchdog\",\"description\":\"watchdog status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/power\",\"description\":\"power status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/config\",\"description\":\"runtime configuration\"},"
        "{\"method\":\"GET\",\"path\":\"/api/logs\",\"description\":\"runtime logs\"},"
        "{\"method\":\"POST\",\"path\":\"/api/config\",\"description\":\"update configuration\"},"
        "{\"method\":\"GET\",\"path\":\"/config/device\",\"description\":\"device configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/wifi\",\"description\":\"wifi configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/watchdog\",\"description\":\"watchdog configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/relay\",\"description\":\"relay configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/tuya\",\"description\":\"tuya configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/security\",\"description\":\"security configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/logs\",\"description\":\"runtime logs page\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/on\",\"description\":\"turn power on\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/off\",\"description\":\"turn power off\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/restart\",\"description\":\"restart power output\"},"
        "{\"method\":\"GET\",\"path\":\"/health\",\"description\":\"liveness\"}"
        "]"
        "}");
}

void WebServerService::handleApiSystem()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeSystem(
            App.status().system,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"system_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiSystemRestart()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (!m_restartRequested)
    {
        m_restartRequested = true;
        m_restartAt = millis() + 500;
    }

    sendJson(
        202,
        "{\"ok\":true,\"command\":\"esp_restart\",\"delayMs\":500}");
}

void WebServerService::handleApiNetwork()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeNetwork(
            App.status().network,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"network_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiHealth()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeHealth(
            App.status().health,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"health_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiWatchdog()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeWatchdog(
            App.status().watchdog,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"watchdog_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiPower()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializePower(
            App.status().power,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"power_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiConfig()
{
    const AppConfig& config =
        Config.data();

    char maskedPassword[WIFI_PASSWORD_LENGTH] {};
    char maskedLocalKey[TUYA_LOCAL_KEY_LENGTH] {};
    char chunk[160] {};

    maskSecret(
        config.wifi.password,
        maskedPassword,
        sizeof(maskedPassword));

    maskSecret(
        config.tuya.localKey,
        maskedLocalKey,
        sizeof(maskedLocalKey));

    m_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    m_server.send(
        200,
        "application/json",
        "");

    snprintf(
        chunk,
        sizeof(chunk),
        "{\"version\":%u,\"device\":{\"hostname\":\"",
        CONFIG_VERSION);

    m_server.sendContent(chunk);
    sendJsonEscaped(m_server, config.device.hostname);

    m_server.sendContent("\"},\"wifi\":{\"ssid\":\"");
    sendJsonEscaped(m_server, config.wifi.ssid);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"passwordSet\":%s,\"passwordMasked\":\"",
        strlen(config.wifi.password) > 0 ? "true" : "false");

    m_server.sendContent(chunk);
    sendJsonEscaped(m_server, maskedPassword);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"reconnectInterval\":%lu,\"connectTimeout\":%lu}",
        static_cast<unsigned long>(config.wifi.reconnectInterval),
        static_cast<unsigned long>(config.wifi.connectTimeout));

    m_server.sendContent(chunk);

    m_server.sendContent(",\"watchdog\":{\"targetHost\":\"");
    sendJsonEscaped(m_server, config.watchdog.targetHost);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"targetPort\":%u,\"pingInterval\":%lu,\"pingTimeout\":%lu,"
        "\"failCount\":%u",
        config.watchdog.targetPort,
        static_cast<unsigned long>(config.watchdog.pingInterval),
        static_cast<unsigned long>(config.watchdog.pingTimeout),
        config.watchdog.failCount);

    m_server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"bootDelay\":%lu,\"powerOffTime\":%lu,\"maxRestartPerDay\":%u}",
        static_cast<unsigned long>(config.watchdog.bootDelay),
        static_cast<unsigned long>(config.watchdog.powerOffTime),
        config.watchdog.maxRestartPerDay);

    m_server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"relay\":{\"enabled\":%s,\"pin\":%u,\"activeHigh\":%s}",
        config.relay.enabled ? "true" : "false",
        config.relay.pin,
        config.relay.activeHigh ? "true" : "false");

    m_server.sendContent(chunk);

    m_server.sendContent(",\"tuya\":{\"ip\":\"");
    sendJsonEscaped(m_server, config.tuya.ipAddress);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"port\":%u,\"deviceId\":\"",
        config.tuya.port);

    m_server.sendContent(chunk);
    sendJsonEscaped(m_server, config.tuya.deviceId);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"localKeySet\":%s,\"localKeyMasked\":\"",
        strlen(config.tuya.localKey) > 0 ? "true" : "false");

    m_server.sendContent(chunk);
    sendJsonEscaped(m_server, maskedLocalKey);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"version\":%u,\"relayDps\":%u,\"statusPollingEnabled\":%s,"
        "\"statusPollingInterval\":%lu}",
        config.tuya.protocolVersion,
        config.tuya.relayDps,
        config.tuya.statusPollingEnabled ? "true" : "false",
        static_cast<unsigned long>(config.tuya.statusPollingInterval));

    m_server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"security\":{\"apiAuthEnabled\":%s,\"apiTokenSet\":%s}}",
        config.security.apiAuthEnabled ? "true" : "false",
        strlen(config.security.apiToken) > 0 ? "true" : "false");

    m_server.sendContent(chunk);
    m_server.sendContent("");
}

void WebServerService::handleApiConfigUpdate()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (!m_server.hasArg("plain"))
    {
        sendJson(
            400,
            "{\"ok\":false,\"error\":\"empty_body\"}");
        return;
    }

    const String body =
        m_server.arg("plain");

    char error[48] {};

    if (!Config.updateFromJson(
            body.c_str(),
            body.length(),
            error,
            sizeof(error)))
    {
        snprintf(
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            "{\"ok\":false,\"error\":\"%s\"}",
            error[0] != '\0' ? error : "update_failed");

        sendJson(
            400,
            m_jsonBuffer);
        return;
    }

    snprintf(
        m_jsonBuffer,
        sizeof(m_jsonBuffer),
        "{\"ok\":true,\"message\":\"configuration_updated\",\"restartRecommended\":true}");

    sendJson(
        200,
        m_jsonBuffer);
}

void WebServerService::handleApiLogs()
{
    const uint8_t count =
        Log.count();

    char chunk[96] {};

    m_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    m_server.send(
        200,
        "application/json",
        "");

    snprintf(
        chunk,
        sizeof(chunk),
        "{\"capacity\":%u,\"count\":%u,\"entries\":[",
        Logger::LOG_CAPACITY,
        count);

    m_server.sendContent(chunk);

    for (uint8_t i = 0; i < count; ++i)
    {
        LogEntry entry;

        if (!Log.entry(
                i,
                entry))
        {
            continue;
        }

        snprintf(
            chunk,
            sizeof(chunk),
            "%s{\"timestamp\":%lu,\"level\":%u,\"levelText\":\"",
            i == 0 ? "" : ",",
            static_cast<unsigned long>(entry.timestamp),
            static_cast<unsigned int>(entry.level));

        m_server.sendContent(chunk);

        sendJsonEscaped(
            m_server,
            entry.levelText);

        m_server.sendContent("\",\"message\":\"");

        sendJsonEscaped(
            m_server,
            entry.message);

        m_server.sendContent("\"}");
    }

    m_server.sendContent("]}");
    m_server.sendContent("");
}

void WebServerService::handleApiPowerOn()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (Power.restartInProgress())
    {
        sendJson(
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    if (!Power.available())
    {
        sendJson(
            503,
            "{\"ok\":false,\"error\":\"power_controller_unavailable\"}");
        return;
    }

    if (!Power.powerOn())
    {
        sendJson(
            500,
            "{\"ok\":false,\"error\":\"power_on_failed\"}");
        return;
    }

    sendJson(
        200,
        "{\"ok\":true,\"command\":\"power_on\"}");
}

void WebServerService::handleApiPowerOff()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (Power.restartInProgress())
    {
        sendJson(
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    if (!Power.available())
    {
        sendJson(
            503,
            "{\"ok\":false,\"error\":\"power_controller_unavailable\"}");
        return;
    }

    if (!Power.powerOff())
    {
        sendJson(
            500,
            "{\"ok\":false,\"error\":\"power_off_failed\"}");
        return;
    }

    sendJson(
        200,
        "{\"ok\":true,\"command\":\"power_off\"}");
}

void WebServerService::handleApiPowerRestart()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (Power.restartInProgress())
    {
        sendJson(
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    const uint32_t powerOffTime = requestedPowerOffTime();

    if (!Power.restart(
            powerOffTime,
            RestartReason::ManualCommand))
    {
        sendJson(
            503,
            "{\"ok\":false,\"error\":\"restart_not_started\"}");
        return;
    }

    snprintf(
        m_jsonBuffer,
        sizeof(m_jsonBuffer),
        "{\"ok\":true,\"command\":\"restart\",\"powerOffTime\":%lu}",
        static_cast<unsigned long>(powerOffTime));

    sendJson(
        202,
        m_jsonBuffer);
}

void WebServerService::handleApiOptions()
{
    sendJson(
        204,
        "");
}

bool WebServerService::authorizeCommand()
{
    const auto& security =
        Config.data().security;

    if (!security.apiAuthEnabled ||
        security.apiToken[0] == '\0')
    {
        return true;
    }

    String headerToken =
        m_server.header("Authorization");

    if (headerToken.startsWith("Bearer "))
    {
        headerToken =
            headerToken.substring(7);
    }

    if (tokenMatches(
            headerToken.c_str(),
            security.apiToken))
    {
        return true;
    }

    if (m_server.hasArg("token") &&
        tokenMatches(
            m_server.arg("token").c_str(),
            security.apiToken))
    {
        return true;
    }

    Log.warning(
        "WebServer: unauthorized command, uri=%s",
        m_server.uri().c_str());

    sendUnauthorized();

    return false;
}

void WebServerService::sendUnauthorized()
{
    sendJson(
        401,
        "{\"ok\":false,\"error\":\"unauthorized\"}");
}

void WebServerService::handleHealth()
{
    sendJson(
        200,
        "{\"status\":\"ok\"}");
}

void WebServerService::handleConfigPage()
{
    handleRoot();
}

void WebServerService::handleLogsPage()
{
    handleRoot();
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
        "Access-Control-Allow-Methods",
        "GET, POST, OPTIONS");

    m_server.sendHeader(
        "Access-Control-Allow-Headers",
        "Content-Type, Authorization");

    m_server.sendHeader(
        "Cache-Control",
        "no-store");

    m_server.send(
        statusCode,
        "application/json",
        json != nullptr ? json : "{}");
}

uint32_t WebServerService::requestedPowerOffTime()
{
    uint32_t powerOffTime =
        Config.data().watchdog.powerOffTime;

    if (m_server.hasArg("powerOffTime"))
    {
        const uint32_t requested =
            static_cast<uint32_t>(
                m_server.arg("powerOffTime").toInt());

        if (requested >= 1000 &&
            requested <= 60000)
        {
            powerOffTime = requested;
        }
    }

    return powerOffTime;
}

void WebServerService::maskSecret(
    const char* source,
    char* output,
    size_t outputSize) const
{
    if (output == nullptr ||
        outputSize == 0)
    {
        return;
    }

    output[0] = '\0';

    if (source == nullptr)
    {
        return;
    }

    const size_t length =
        strlen(source);

    const size_t capacity =
        outputSize - 1;

    const size_t copyLength =
        length < capacity
            ? length
            : capacity;

    for (size_t i = 0; i < copyLength; ++i)
    {
        if (copyLength > 4 &&
            i < (copyLength - 4))
        {
            output[i] = '*';
        }
        else
        {
            output[i] = source[i];
        }
    }

    output[copyLength] = '\0';
}

bool WebServerService::tokenMatches(
    const char* received,
    const char* expected) const
{
    if (received == nullptr ||
        expected == nullptr ||
        received[0] == '\0' ||
        expected[0] == '\0')
    {
        return false;
    }

    const size_t receivedLength =
        strlen(received);

    const size_t expectedLength =
        strlen(expected);

    if (receivedLength != expectedLength ||
        receivedLength >= API_TOKEN_LENGTH)
    {
        return false;
    }

    uint8_t diff = 0;

    for (size_t i = 0; i < expectedLength; ++i)
    {
        const char a = received[i];
        const char b = expected[i];

        diff |= static_cast<uint8_t>(a ^ b);
    }

    return diff == 0;
}

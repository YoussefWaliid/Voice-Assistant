#pragma once
#include <Arduino.h>

// واجهة الويب (في ملف منفصل عشان Arduino preprocessor بيلخبط مع الـ raw string الطويلة)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl" lang="ar">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="theme-color" content="#0f172a">
<link rel="manifest" href="/manifest.webmanifest">
<link rel="icon" type="image/png" href="/icon-192.png">
<link rel="apple-touch-icon" href="/icon-192.png">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-title" content="Voice Hub">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<title>Voice Hub</title>
<script>(function(){var t=null;try{t=localStorage.getItem('theme')}catch(e){}if(t!=='dark'&&t!=='light')t=matchMedia('(prefers-color-scheme:dark)').matches?'dark':'light';document.documentElement.dataset.theme=t;})();</script>
<style>
:root{--bg:#f1f5f9;--card:#fff;--text:#0f172a;--mute:#64748b;--line:#e2e8f0;--pri:#2563eb;--pri2:#1d4ed8;--ok:#16a34a;--bad:#dc2626;--chip:#eef2ff;--term:#0b1220;--termtx:#cbd5e1}
:root[data-theme="dark"]{color-scheme:dark;--bg:#0b1220;--card:#131c2e;--text:#e5e7eb;--mute:#94a3b8;--line:#22304a;--pri:#3b82f6;--pri2:#2563eb;--chip:#1b2740;--term:#060b16}
*{box-sizing:border-box}
[hidden]{display:none!important}
body{margin:0;background:var(--bg);color:var(--text);font-family:'Segoe UI',Tahoma,'Noto Sans Arabic',sans-serif;font-size:15px}
.wrap{max-width:720px;margin:0 auto;padding:16px}
header{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:14px}
header h1{font-size:20px;margin:0}
.live{display:flex;align-items:center;gap:6px;font-size:12px;color:var(--mute)}
.dot{width:9px;height:9px;border-radius:50%;background:var(--mute)}
.dot.on{background:var(--ok);box-shadow:0 0 0 3px rgba(22,163,74,.2)}
.tabs{display:flex;gap:6px;background:var(--card);padding:5px;border-radius:12px;border:1px solid var(--line);margin-bottom:14px}
.tab{flex:1;border:0;background:none;color:var(--mute);padding:10px;border-radius:8px;font:inherit;font-weight:600;cursor:pointer}
.tab.active{background:var(--pri);color:#fff}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px;margin-bottom:14px}
.card h2{font-size:16px;margin:0 0 12px;display:flex;align-items:center;justify-content:space-between}
.badge{font-size:12px;background:var(--chip);color:var(--pri);padding:2px 9px;border-radius:99px;font-weight:700}
input,select{width:100%;padding:11px 12px;margin-bottom:10px;border:1px solid var(--line);border-radius:9px;background:var(--bg);color:var(--text);font:inherit}
input:focus,select:focus{outline:2px solid var(--pri);outline-offset:-1px}
.btn{border:0;border-radius:9px;padding:10px 14px;font:inherit;font-weight:600;cursor:pointer;background:var(--pri);color:#fff;transition:.15s}
.btn:hover{background:var(--pri2)}
.btn:disabled{opacity:.6;cursor:wait}
.btn.full{width:100%}
.btn.ghost{background:var(--chip);color:var(--text)}
.btn.ghost:hover{background:var(--line)}
.btn.run{background:var(--ok)}
.btn.del{background:none;color:var(--bad);border:1px solid var(--bad)}
.btn.del:hover{background:var(--bad);color:#fff}
.btn.danger{background:var(--bad)}
.cmd{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:12px;border:1px solid var(--line);border-radius:11px;margin-bottom:9px}
.cmd-main{min-width:0;flex:1}
.cmd-name{font-weight:700;margin-bottom:5px;word-break:break-word}
.cmd-meta{display:flex;align-items:center;gap:8px;min-width:0}
.cmd-meta code{direction:ltr;font-size:12px;color:var(--mute);overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.cmd-actions{display:flex;gap:6px;flex-shrink:0}
.empty{text-align:center;color:var(--mute);padding:22px 0}
.info{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:8px}
.chip{background:var(--chip);border-radius:10px;padding:9px 11px}
.chip small{display:block;color:var(--mute);font-size:11px}
.chip b{direction:ltr;display:inline-block;font-size:14px}
.tools{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:10px;align-items:center}
.tools .btn{padding:7px 11px;font-size:13px}
.tools label{font-size:13px;color:var(--mute);display:flex;align-items:center;gap:5px;margin-inline-start:auto}
.tools input{width:auto;margin:0}
#term{direction:ltr;text-align:left;background:var(--term);color:var(--termtx);border-radius:10px;padding:10px;height:min(340px,50vh);overflow-y:auto;font:12.5px/1.55 ui-monospace,Consolas,Menlo,monospace}
#term div{white-space:pre-wrap;word-break:break-all}
#term .t{color:#475569;margin-right:8px;white-space:nowrap;font-variant-numeric:tabular-nums}
#term .err{color:#f87171}#term .ok{color:#4ade80}#term .wake{color:#22d3ee}#term .proc{color:#c084fc}#term .sys{color:#fbbf24}
.send{display:flex;gap:6px;margin-top:10px}
.send input{margin:0}
.send .btn{white-space:nowrap}
textarea{width:100%;padding:11px 12px;margin-bottom:10px;border:1px solid var(--line);border-radius:9px;background:var(--bg);color:var(--text);font:inherit;resize:vertical}
textarea:focus{outline:2px solid var(--pri);outline-offset:-1px}
.lbl{display:block;font-size:13px;color:var(--mute);margin:2px 0 6px}
.step{display:flex;gap:6px;align-items:flex-start}
.step select{width:90px;flex-shrink:0}
.step .btn{padding:10px 12px}
.cmd-link{display:flex;align-items:center;gap:6px;margin-top:8px;min-width:0}
.cmd-link input{margin:0;direction:ltr;flex:1;min-width:0;font-size:12px;font-family:monospace;color:var(--mute);background:var(--chip);border:1px solid var(--line,transparent);border-radius:8px;padding:6px 8px}
.cmd-link .btn{padding:6px 10px;font-size:13px;flex:none}
.phrases{display:flex;flex-wrap:wrap;gap:5px;margin-bottom:6px}
.ph{font-size:12px;background:var(--chip);padding:2px 8px;border-radius:99px}
.step select.irsel{width:auto;flex:1;min-width:0}
.irstat{margin:10px 0;color:var(--mute);font-size:14px}
.irrow{display:flex;gap:8px}.irrow>*{flex:1}
.step .sv{display:flex;gap:6px;flex:1;min-width:0}.step .sv select{width:auto;flex:1;min-width:0}
.step .dl{display:flex;gap:6px;flex:1;min-width:0;align-items:center}.step .dl span{color:var(--mute);font-size:13px;white-space:nowrap;margin-bottom:10px}
.panel.sl{animation:slin .22s ease}
@keyframes slin{from{opacity:0;transform:translateX(var(--sx,30px))}to{opacity:1;transform:none}}
/* ---- تاب المشترك (شكل عصري) ---- */
#tab-strip{--g:#16a34a;--gs:rgba(22,163,74,.10);--mono:ui-monospace,Consolas,'SFMono-Regular',monospace}
:root[data-theme="dark"] #tab-strip{--g:#4ade80;--gs:rgba(74,222,128,.08)}
.hero,.scard{background:var(--card);border:1px solid var(--line);border-radius:24px}
.hero{overflow:hidden;margin-bottom:14px}
.hero .top{display:flex;justify-content:space-between;align-items:flex-start;padding:18px 20px 14px;gap:10px}
.hero .lbl{color:var(--mute);font-size:14px}
.hero .big{font-family:var(--mono);font-size:48px;font-weight:800;color:var(--g);direction:ltr;line-height:1.1;margin-top:6px;text-align:left}
.hero .big small{font-size:18px;color:var(--mute);font-weight:500;margin-inline-start:6px}
.lv{display:flex;align-items:center;gap:8px;font:700 13px var(--mono);letter-spacing:.12em;color:var(--g);direction:ltr}
.lv i{width:11px;height:11px;border-radius:50%;background:var(--g);box-shadow:0 0 0 4px var(--gs);animation:lvp 1.6s infinite}
.lv.off{color:var(--mute)}.lv.off i{background:var(--mute);box-shadow:none;animation:none}
@keyframes lvp{50%{box-shadow:0 0 0 8px transparent}}
.hstats{display:grid;grid-template-columns:repeat(3,1fr);border-top:1px solid var(--line)}
.hstats>div{padding:12px 16px;border-inline-start:1px solid var(--line);min-width:0}
.hstats>div:first-child{border-inline-start:0}
.hstats b{display:block;font:700 19px var(--mono);direction:ltr;text-align:start}
.hstats span{font-size:12px;color:var(--mute)}
.scard{padding:18px}
.shead{display:flex;align-items:center;gap:10px}
.shead h3{margin:0;font-size:20px}
.shead .d{width:11px;height:11px;border-radius:50%;background:var(--mute)}
.scard.on .shead .d{background:var(--g);box-shadow:0 0 10px var(--g)}
.pill{border:1px solid var(--g);color:var(--g);border-radius:99px;padding:3px 12px;font-size:13px;font-weight:700}
.pill.off{border-color:var(--line);color:var(--mute)}
.gear{margin-inline-start:auto;background:none;border:0;color:var(--mute);font-size:22px;cursor:pointer;padding:4px 8px}
.ssum{font-family:var(--mono);color:var(--g);direction:ltr;text-align:left;margin:14px 2px 16px;font-size:15px}
.stg{position:relative}
.sgrid2{display:grid;grid-template-columns:1fr 1fr;gap:12px;direction:ltr}
.tile{position:relative;display:flex;flex-direction:column;gap:3px;padding:14px;min-height:150px;border:1.5px solid var(--line);border-radius:22px;background:var(--bg);cursor:pointer;user-select:none;transition:.2s;-webkit-tap-highlight-color:transparent}
.tile.on{border-color:var(--g);background:var(--gs)}
.tile.dis{opacity:.55;cursor:not-allowed}
.tile.bad{border-color:var(--bad)}
.tile:active:not(.dis){transform:scale(.97)}
.tile:nth-child(even){align-items:flex-end;text-align:right}
.tile:nth-child(odd){align-items:flex-start;text-align:left}
.tile .pg{width:50px;height:50px;color:var(--mute);margin-bottom:auto;transition:.2s}
.tile.on .pg{color:var(--g)}
.tile .nm{font-size:16px;font-weight:700;max-width:100%;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;direction:rtl;margin-top:10px}
.tile .w{font:700 17px var(--mono);color:var(--mute);direction:ltr}
.tile.on .w{color:var(--g)}
.tile .sub{font:12px var(--mono);color:var(--mute);direction:ltr}
.tile .a{font:600 13px var(--mono);color:var(--mute);direction:ltr}
.tile.on .a{color:var(--g);opacity:.85}
.hero .big{display:flex;flex-wrap:wrap;align-items:baseline}
.hero .big small.amp{color:var(--g);font-family:var(--mono);font-weight:700;margin-inline-start:16px}
.tile .ed{position:absolute;top:10px;background:none;border:0;color:var(--mute);font-size:14px;cursor:pointer;padding:4px}
.tile:nth-child(odd) .ed{right:8px}.tile:nth-child(even) .ed{left:8px}
.tile .wn{position:absolute;bottom:10px;font-size:14px}
.tile:nth-child(odd) .wn{right:10px}.tile:nth-child(even) .wn{left:10px}
.hub{position:absolute;top:50%;left:50%;width:58px;height:58px;margin:-29px 0 0 -29px;border-radius:50%;border:2px solid var(--line);background:var(--card);color:var(--mute);box-shadow:0 0 0 7px var(--card);cursor:pointer;display:flex;align-items:center;justify-content:center;transition:.2s;z-index:2}
.hub.on{color:var(--g);border-color:var(--g);box-shadow:0 0 0 7px var(--card),0 0 18px var(--gs)}
.hub:disabled{opacity:.5;cursor:not-allowed}
.sdet{margin-top:16px;padding-top:14px;border-top:1px solid var(--line)}
.sgrid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.so{border:1px solid var(--line);border-radius:12px;padding:12px;background:var(--bg)}
.so.on{border-color:var(--ok);box-shadow:0 0 0 2px rgba(22,163,74,.15)}
.so h3{margin:0 0 6px;font-size:14px;display:flex;justify-content:space-between;align-items:center}
.so .m{font-size:12px;color:var(--mute);margin-bottom:8px;direction:ltr;text-align:left}
.so .btn{width:100%;padding:11px}
.so.bad{border-color:var(--bad)}
.sinfo{font-size:13px;color:var(--mute);line-height:1.8;direction:ltr;text-align:left}
#toast{position:fixed;bottom:22px;left:50%;transform:translate(-50%,30px);background:#0f172a;color:#fff;padding:10px 18px;border-radius:99px;opacity:0;pointer-events:none;transition:.25s;font-size:14px;z-index:9}
#toast.show{opacity:1;transform:translate(-50%,0)}
#toast.bad{background:var(--bad)}
.step .mv{display:flex;flex-direction:column;gap:2px;flex-shrink:0}
.step .mv .btn{padding:0 8px;height:20px;line-height:1;font-size:11px;margin:0}
.step .grip{cursor:grab;color:var(--mute);font-size:18px;padding:10px 4px;user-select:none;touch-action:none}
.step.dragging{opacity:.4}
.step.over{outline:2px dashed var(--pri);outline-offset:2px;border-radius:9px}
.bk{display:flex;gap:8px;margin-bottom:10px}.bk>*{flex:1}
.hr{display:flex;align-items:center;gap:12px}
.thm{padding:6px 10px;font-size:16px;line-height:1}
.cath{cursor:pointer;user-select:none}
.catt{display:flex;align-items:center;gap:8px;min-width:0}
.catt .carr{color:var(--mute);width:14px;text-align:center}
.cmv{display:flex;gap:4px;flex-shrink:0}
.cmv .btn{padding:3px 9px;font-size:12px}
.cmv .btn:disabled{opacity:.35;cursor:default}
.catbody .cmd:last-child{margin-bottom:0}
.tabs{position:sticky;top:6px;z-index:5;box-shadow:0 2px 8px rgba(0,0,0,.08)}
@media(min-width:900px){.wrap{max-width:860px}}
.badge.ok{background:rgba(22,163,74,.15);color:var(--ok)}
.badge.bad{background:rgba(220,38,38,.15);color:var(--bad)}
.srch{display:flex;gap:6px;margin-bottom:14px}.srch input{margin:0;flex:1}.srch .btn{padding:8px 12px;flex:none}
.cmd-name.tg{cursor:pointer}
.cmd-name .chev{color:var(--mute);font-size:12px;margin-inline-start:6px}
.stl{margin:8px 0;border:1px solid var(--line);border-radius:10px;overflow:hidden}
.stp{display:flex;align-items:center;gap:8px;padding:6px 10px;font-size:13px}
.stp+.stp{border-top:1px solid var(--line)}
.stp .n{flex:none;width:20px;height:20px;border-radius:50%;background:var(--chip);color:var(--pri);font-size:11px;font-weight:700;display:flex;align-items:center;justify-content:center}
.stp .ty{flex:none;min-width:54px;text-align:center;font-size:11px;font-weight:700;background:var(--chip);color:var(--pri);border-radius:6px;padding:2px 6px}
.stp .v{flex:1;min-width:0;text-align:right;word-break:break-all;font-size:12.5px}
.so h3 .nm{cursor:pointer}
.so h3 .nm small{color:var(--mute)}
.tools select{width:auto;margin:0;padding:7px 9px;font-size:13px}
.tools input[type=search]{width:auto;flex:1;min-width:120px;margin:0;padding:7px 10px;font-size:13px}

/* ================= موبايل ================= */
html{-webkit-text-size-adjust:100%}
.tab{white-space:nowrap}
.btn{touch-action:manipulation}
.tile .ed{padding:8px}
.tile .wn{bottom:auto;top:12px}
.tile:nth-child(odd) .wn{right:40px}.tile:nth-child(even) .wn{left:40px}
@media(max-width:600px){
  body{font-size:14.5px}
  .wrap{padding:10px 10px 28px}
  header{margin-bottom:10px}
  header h1{font-size:18px}
  .tabs{gap:3px;padding:4px;top:4px;border-radius:12px;margin-bottom:10px}
  .tab{padding:10px 2px;font-size:13.5px}
  .card{padding:12px;border-radius:12px;margin-bottom:10px}
  .card h2{font-size:15px;margin-bottom:10px}
  input,select,textarea{font-size:16px}
  /* قائمة الأوامر والإشارات: الأزرار تحت المحتوى بعرض كامل بدل ما تاكل العرض */
  .cmd{flex-direction:column;align-items:stretch;gap:10px;padding:10px}
  .cmd.hrow{flex-direction:row;align-items:center}
  .cmd-actions{width:100%}
  .cmd.hrow .cmd-actions{width:auto}
  .cmd-actions .btn{flex:1;padding:10px 0}
  .cmd.hrow .cmd-actions .btn{flex:none;padding:10px 16px}
  .cmd-link .btn{padding:8px 10px}
  .stp{padding:6px 8px}
  /* صف الخطوة في الفورم: سطر للنوع + سطر للقيمة */
  .step{flex-wrap:wrap;gap:6px}
  .step .grip{order:1;padding:8px 2px}
  .step .mv{order:2}
  .step .mv .btn{height:22px;padding:0 10px}
  .step .tsel{order:3;flex:1;width:auto;min-width:0}
  .step>.btn.del{order:4}
  .step .tin,.step .irsel,.step .sv,.step .dl{order:5;flex:1 1 100%;width:100%}
  .step .sv select{min-width:0}
  .send .btn{padding:10px 12px}
  .tools .btn{padding:8px 10px}
  .tools label{margin-inline-start:0;width:100%}
  #term{height:min(300px,45vh);font-size:12px}
  .info{grid-template-columns:repeat(2,1fr)}
  .irrow{flex-direction:column}
  /* تاب المشترك */
  .hero{border-radius:20px;margin-bottom:10px}
  .hero .top{padding:14px 14px 10px}
  .hero .lbl{font-size:13px}
  .hero .big{font-size:38px}
  .hero .big small{font-size:15px}
  .lv{font-size:11px}
  .hstats>div{padding:10px}
  .hstats b{font-size:15px}
  .hstats span{font-size:11px}
  .scard{padding:12px;border-radius:20px}
  .shead h3{font-size:18px}
  .ssum{font-size:13.5px;margin:10px 2px 12px}
  .sgrid2{gap:10px}
  .tile{min-height:128px;padding:12px;border-radius:18px}
  .tile .pg{width:42px;height:42px}
  .tile .nm{font-size:15px;margin-top:8px}
  .tile .w{font-size:16px}
  .tile .sub{font-size:11px;white-space:normal}
  .hub{width:52px;height:52px;margin:-26px 0 0 -26px;box-shadow:0 0 0 6px var(--card)}
  .hub.on{box-shadow:0 0 0 6px var(--card),0 0 16px var(--gs)}
  .sgrid{gap:8px}
  #toast{width:max-content;max-width:90vw;text-align:center;bottom:16px}
}
@media(max-width:360px){
  .tab{font-size:12.5px}
  .hero .big{font-size:34px}
  .tile .nm{font-size:14px}
}
.subtabs{display:flex;gap:6px;overflow-x:auto;padding:2px 2px 10px;margin-bottom:4px;scrollbar-width:none}
.subtabs::-webkit-scrollbar{display:none}
.stab{flex:none;border:1px solid var(--line);background:var(--card);color:var(--mute);padding:8px 14px;border-radius:99px;font:inherit;font-size:13.5px;font-weight:600;cursor:pointer;white-space:nowrap}
.stab.active{background:var(--pri);border-color:var(--pri);color:#fff}
</style>
</head>
<body>
<div class="wrap">
  <header>
    <h1>🎙️ Voice Hub</h1>
    <div class="hr"><div class="live"><span class="dot" id="dot"></span><span id="ip">--</span></div><button class="btn ghost thm" id="instBtn" onclick="installApp()" hidden title="تثبيت كتطبيق">📲</button><button class="btn ghost thm" id="themeBtn" onclick="toggleTheme()" title="تغيير الثيم">🌙</button></div>
  </header>

  <div class="tabs">
    <button class="tab active" data-tab="cmds" onclick="showTab('cmds')">الأوامر</button>
    <button class="tab" data-tab="strip" onclick="showTab('strip')">المشترك</button>
    <button class="tab" data-tab="maint" onclick="showTab('maint')">الصيانة</button>
  </div>

  <section class="panel" id="tab-cmds">
    <div class="card" id="form-card" data-cl="form:0">
      <h2><span id="form-title">إضافة أمر جديد</span><button class="btn ghost" id="cancelBtn" onclick="resetForm()" hidden>إلغاء التعديل</button></h2>
      <input type="text" id="name" placeholder="اسم الأمر (مثال: نور السقف)">
      <input type="text" id="cat" list="cats" placeholder="التصنيف (مثال: إضاءة) - اختياري">
      <datalist id="cats"></datalist>
      <label class="lbl">الجمل المنطوقة (جملة في كل سطر - كلها بتنفذ نفس الأمر)</label>
      <textarea id="phrases" rows="3" dir="auto" placeholder="شغل نور السقف&#10;شغلي نور السقف&#10;turn on ceiling light"></textarea>
      <label class="lbl">الخطوات (بتتنفذ بالترتيب)</label>
      <div id="steps"></div>
      <button class="btn ghost full" onclick="addStep()">＋ إضافة خطوة</button>
      <button class="btn full" style="margin-top:10px" onclick="saveCommand()">حفظ الأمر</button>
    </div>

    <div class="card">
      <h2 id="heardHead" style="cursor:pointer;user-select:none" onclick="toggleHeard()"><span>آخر ما سمعه الجهاز</span><span id="heardArrow">◂</span></h2>
      <div id="heardBody" hidden>
        <div class="tools"><button class="btn ghost" onclick="loadHeard()">تحديث</button> <button class="btn ghost" onclick="clearHeard()">🧹 مسح</button><label><input type="checkbox" id="heardOn" onchange="setHeardOn()"> تسجيل ما يسمعه الجهاز (وطباعته في السيريال)</label></div>
        <div class="irstat" style="margin-top:0">لو سمع كلمة غلط، اضغط ＋ تضيفها كجملة للأمر (بتتحط في خانة الجمل فوق، اختار الأمر واحفظ).</div>
        <div id="heard"></div>
      </div>
    </div>

    <div class="srch"><input type="search" id="q" dir="auto" placeholder="🔍 ابحث في الأوامر والجمل والتصنيفات..." oninput="renderCommands()"><button class="btn ghost" onclick="setAllCats(true)" title="فتح الكل">⇊</button><button class="btn ghost" onclick="setAllCats(false)" title="قفل الكل">⇈</button></div>
    <div id="groups"></div>

    <div class="card">
      <h2>كل الأوامر <span class="badge" id="cmd-count">0</span></h2>
      <button class="btn danger full" onclick="clearAllCommands()">مسح كل الأوامر</button>
    </div>
  </section>

  <section class="panel" id="tab-strip" hidden>
    <div class="hero">
      <div class="top">
        <div>
          <div class="lbl" id="st-lbl">الاستهلاك الآن</div>
          <div class="big"><span id="st-w">0.0</span><small>W</small><small class="amp" id="st-a">0.00 A</small></div>
        </div>
        <div class="lv off" id="st-live"><i></i><span id="st-livetx">OFFLINE</span></div>
      </div>
      <div class="hstats">
        <div><b id="st-cnt">0/4</b><span>مخارج شغالة</span></div>
        <div><b id="st-kwh">0.000</b><span>kWh إجمالي</span></div>
        <div><b id="st-age">--</b><span>آخر قراءة</span></div>
      </div>
    </div>

    <div class="scard" id="st-card">
      <div class="shead">
        <span class="d"></span><h3>المشترك</h3><span class="pill off" id="st-badge">غير متصل</span>
        <button class="gear" onclick="$('#st-det').hidden=!$('#st-det').hidden" title="تفاصيل">⚙</button>
      </div>
      <div class="ssum" id="st-sum">-- W</div>
      <div class="stg">
        <div class="sgrid2" id="st-grid"></div>
        <button class="hub" id="st-hub" onclick="stripHub()" title="تشغيل/إطفاء الكل"><svg viewBox="0 0 24 24" width="26" height="26" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round"><path d="M12 3v8"/><path d="M6.3 7.2a8 8 0 1 0 11.4 0"/></svg></button>
      </div>
      <button class="btn ghost full" style="margin-top:12px" onclick="stripRefresh(true)">🔄 تحديث القراءات</button>
      <div class="sdet" id="st-det" hidden>
        <div class="sinfo" id="st-info">--</div>
        <div class="sinfo" style="margin-top:8px;direction:rtl;text-align:right">لإضافة/إعادة ربط المشترك: من تطبيق MTTL Control &gt; Device setup اكتب IP الجهاز ده: <b id="st-ip">--</b> (بورت 10086)</div>
        <button class="btn ghost full" style="margin-top:10px" onclick="stripRaw()">🔍 عرض الرد الخام (تشخيص)</button>
        <pre id="st-raw" hidden style="direction:ltr;text-align:left;white-space:pre-wrap;word-break:break-all;font:12px/1.6 ui-monospace,monospace;background:var(--bg);border:1px solid var(--line);border-radius:10px;padding:10px;margin:10px 0 0;user-select:text"></pre>
      </div>
    </div>
  </section>

  <section class="panel" id="tab-maint" hidden>
    <div class="subtabs" id="subtabs"><button class="stab active" data-sub="log" onclick="showSub('log')">السجل</button><button class="stab" data-sub="dev" onclick="showSub('dev')">الجهاز</button><button class="stab" data-sub="ir" onclick="showSub('ir')">IR</button><button class="stab" data-sub="mic" onclick="showSub('mic')">الميكروفون</button><button class="stab" data-sub="bk" onclick="showSub('bk')">النسخ الاحتياطي</button></div>
    <div class="sub" id="sub-dev" hidden>
    <div class="card" data-cl="m_info:1">
      <h2>حالة الجهاز</h2>
      <div class="info" id="info"></div>
    </div>
    <div class="card" data-cl="pwa:1">
      <h2>تثبيت كتطبيق <span class="badge" id="pwa-badge">--</span></h2>
      <div class="sinfo" id="pwa-info" style="direction:rtl;text-align:right">--</div>
      <button class="btn full" id="pwa-btn" style="margin-top:10px" onclick="installApp()" hidden>📲 تثبيت التطبيق</button>
    </div>
    <div class="card" data-cl="m_tools:1">
      <h2>أدوات</h2>
      <button class="btn danger full" onclick="reboot()">🔄 إعادة تشغيل الجهاز</button>
    </div>
  
    </div>
    <div class="sub" id="sub-ir" hidden>
    <div class="card" data-cl="learn:1">
      <h2>تعلّم إشارة جديدة</h2>
      <button class="btn full" id="learnBtn" onclick="startLearn()">🎯 ابدأ التعلم</button>
      <div class="irstat" id="irstat"></div>
      <div id="irnew" hidden>
        <input type="text" id="irname" placeholder="اسم الإشارة (مثال: تشغيل البروجيكتور)">
        <div class="irrow">
          <select id="irkhz"><option value="38">38 kHz</option><option value="36">36 kHz</option><option value="40">40 kHz</option><option value="56">56 kHz</option></select>
          <select id="irrep"><option value="1">إرسال مرة</option><option value="2">مرتين</option><option value="3">3 مرات</option></select>
        </div>
        <div class="irrow">
          <button class="btn ghost" onclick="testIr()">▶ جرّب</button>
          <button class="btn" onclick="saveIr()">حفظ</button>
        </div>
      </div>
    </div>
    <div class="card" data-cl="irsaved:1">
      <h2>الإشارات المحفوظة <span class="badge" id="ir-count">0</span></h2>
      <input type="search" id="irq" dir="auto" placeholder="🔍 بحث في الإشارات" oninput="renderIr()" hidden>
      <div id="irlist"></div>
    </div>
  
    <div class="card" data-cl="irpins:1">
      <h2>أطراف الـ IR (GPIO)</h2>
      <label class="lbl">المرسل (IR LED) - TX</label>
      <select id="irtx"></select>
      <label class="lbl">المستقبل (VS1838B) - RX</label>
      <select id="irrx"></select>
      <button class="btn full" onclick="saveIrCfg()">حفظ الأطراف</button>
    </div>
    </div>
    <div class="sub" id="sub-mic" hidden>
    <div class="card" data-cl="m_mic:1">
      <h2>الميكروفون</h2>
      <label class="lbl">تكبير كلمة التنبيه (wake word) - زوّده لو مش بيسمعك من بعيد (2 - 128)</label>
      <input type="number" id="micg" min="2" max="128" step="2" dir="ltr">
      <label class="lbl">أقل مستوى يعتبر كلام أثناء التسجيل - قلله لو بيفوّت كلامك الواطي (2 - 60)</label>
      <input type="number" id="micv" min="2" max="60" step="1" dir="ltr">
      <button class="btn full" onclick="saveMic()">حفظ</button>
      <div class="sinfo" id="micinfo" style="margin-top:10px">--</div>
    </div>
    </div>
    <div class="sub" id="sub-log">
    <div class="card" data-cl="m_log:1">
      <h2>Serial Monitor</h2>
      <div class="tools">
        <button class="btn ghost" id="pauseBtn" onclick="togglePause()">⏸ إيقاف</button>
        <button class="btn ghost" onclick="clearTerm()">🧹 مسح</button>
        <button class="btn ghost" onclick="copyLog()">📋 نسخ</button>
        <button class="btn ghost" onclick="downloadLog()">⬇ تحميل</button>
        <label><input type="checkbox" id="autoscroll" checked> تمرير تلقائي</label>
      </div>
      <div class="tools"><input type="search" id="logq" dir="auto" placeholder="🔍 فلتر السجل..." oninput="applyLogFilter()"><select id="logLvl" onchange="applyLogFilter()"><option value="">الكل</option><option value="err">أخطاء</option><option value="ok">نجاح</option><option value="wake">كلمة التنبيه</option></select></div>
      <div id="term"></div>
      <div class="send">
        <input type="text" id="say" dir="auto" placeholder="جرّب أمر نصي بدل الكلام (مثال: turn on ring light)" onkeydown="if(event.key==='Enter')sendSay()">
        <button class="btn" onclick="sendSay()">إرسال</button>
      </div>
    </div>
    </div>
    <div class="sub" id="sub-bk" hidden>
    <div class="card" data-cl="m_bk:1">
      <h2>نسخة احتياطية للأوامر</h2>
      <div class="bk">
        <button class="btn ghost" onclick="exportCmds()">⬇ تصدير (JSON)</button>
        <button class="btn ghost" onclick="$('#impFile').click()">⬆ استيراد (JSON)</button>
      </div>
      <input type="file" id="impFile" accept=".json,application/json" hidden onchange="importCmds(this)">
      <div class="sinfo">التصدير بيحفظ كل الأوامر (الجمل + الخطوات + الترتيب) في ملف على جهازك. وبيحفظ كمان الإشارات الـ IR المتعلَّمة، وبيتعاد ربطها بالأوامر تلقائي وقت الاستيراد.</div>
    </div>
    </div>
  </section>

</div>
<div id="toast"></div>

<script>
const $ = s => document.querySelector(s);
const el = (tag, cls, txt) => { const e = document.createElement(tag); if (cls) e.className = cls; if (txt !== undefined) e.textContent = txt; return e; };
const post = (url, data) => fetch(url, {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:new URLSearchParams(data||{}).toString()});
let toastT;
function toast(msg, bad) {
  const t = $('#toast'); t.textContent = msg; t.className = 'show' + (bad ? ' bad' : '');
  clearTimeout(toastT); toastT = setTimeout(() => t.className = '', 2400);
}

/* ---------- theme + local UI state ---------- */
const lsGet = (k, d) => { try { const v = JSON.parse(localStorage.getItem(k)); return v == null ? d : v; } catch (e) { return d; } };
const lsSet = (k, v) => { try { localStorage.setItem(k, JSON.stringify(v)); } catch (e) {} };
function applyTheme(t) {
  document.documentElement.dataset.theme = t;
  $('#themeBtn').textContent = t === 'dark' ? '☀️' : '🌙';
  const m = document.querySelector('meta[name=theme-color]'); if (m) m.content = t === 'dark' ? '#0b1220' : '#f1f5f9';
}
function toggleTheme() {
  const t = document.documentElement.dataset.theme === 'dark' ? 'light' : 'dark';
  try { localStorage.setItem('theme', t); } catch (e) {}
  applyTheme(t);
}

/* ---------- collapsible cards ---------- */
const CARDS = {};
function makeCollapsible(card, key, def) {
  const h = card.querySelector(':scope > h2'); if (!h) return;
  const body = el('div', 'cbody');
  [...card.children].forEach(c => { if (c !== h) body.appendChild(c); });
  card.appendChild(body);
  const left = el('span', 'catt'), arrow = el('span', 'carr');
  [...h.childNodes].forEach(n => { if (!(n.nodeType === 1 && n.classList.contains('btn'))) left.appendChild(n); });
  left.prepend(arrow); h.prepend(left); h.classList.add('cath');
  const set = (open, save) => {
    body.hidden = !open; arrow.textContent = open ? '▾' : '◂'; h.style.marginBottom = open ? '' : '0';
    if (save) { const o = lsGet('cardOpen', {}); o[key] = open ? 1 : 0; lsSet('cardOpen', o); }
  };
  const st = lsGet('cardOpen', {});
  set(key in st ? !!st[key] : !!def, false);
  h.onclick = e => { if (e.target.closest('.btn')) return; set(body.hidden, true); };
  CARDS[key] = open => set(open, false);
}
function initCards() { document.querySelectorAll('[data-cl]').forEach(c => { const [k, d] = c.dataset.cl.split(':'); makeCollapsible(c, k, d === '1'); }); }
function openForm() { if (CARDS.form) CARDS.form(true); }

/* ---------- tabs ---------- */
function showSub(name) {
  if (!name) name = 'log';
  if (!document.getElementById('sub-' + name)) name = 'log';
  document.querySelectorAll('.stab').forEach(b => b.classList.toggle('active', b.dataset.sub === name));
  document.querySelectorAll('.sub').forEach(d => d.hidden = d.id !== 'sub-' + name);
  const sb = document.querySelector('.stab.active'); if (sb && sb.scrollIntoView) sb.scrollIntoView({inline: 'center', block: 'nearest'});
  clearInterval(infoTimer); stopLog();
  if (name === 'dev') { loadInfo(); infoTimer = setInterval(loadInfo, 10000); }
  if (name === 'ir') { loadIrCfg(); loadIrList(); }
  if (name === 'mic') loadMic();
  if (name === 'log') startLog();
}
function showTab(name, dir) {
  const ord = [...document.querySelectorAll('.tab')].map(b => b.dataset.tab), pa = document.querySelector('.tab.active'), prev = pa ? pa.dataset.tab : null;
  document.querySelectorAll('.tab').forEach(b => b.classList.toggle('active', b.dataset.tab === name));
  document.querySelectorAll('.panel').forEach(p => p.hidden = p.id !== 'tab-' + name);
  if (name === 'strip') startStrip(); else stopStrip();
  if (name === 'cmds' && !$('#heardBody').hidden) loadHeard();
  if (name === 'maint') { showSub(window.__sub || 'log'); window.__sub = null; } else { stopLog(); clearInterval(infoTimer); }
  const pn = $('#tab-' + name);
  if (prev && prev !== name) { pn.style.setProperty('--sx', (dir || (ord.indexOf(name) > ord.indexOf(prev) ? -1 : 1)) * 30 + 'px'); pn.classList.remove('sl'); void pn.offsetWidth; pn.classList.add('sl'); }
  const at = document.querySelector('.tab.active'); if (at && at.scrollIntoView) at.scrollIntoView({inline: 'center', block: 'nearest'});
}

/* ---------- سوايب بين التابات (RTL: سحب لليسار = التاب اللي بعده) ---------- */
(function () {
  let sx = 0, sy = 0, st = 0, ok = false;
  document.addEventListener('touchstart', e => {
    ok = false; if (e.touches.length !== 1) return;
    const t = e.touches[0], tg = e.target;
    if (t.clientX < 24 || t.clientX > innerWidth - 24) return;           // حواف الشاشة = إيماءة الرجوع
    if (tg.closest('input,textarea,select,.tabs,pre')) return;
    for (let n = tg; n && n !== document.body; n = n.parentElement) {     // عناصر بتتحرك أفقي
      if (n.scrollWidth > n.clientWidth + 4) { const o = getComputedStyle(n).overflowX; if (o === 'auto' || o === 'scroll') return; }
    }
    sx = t.clientX; sy = t.clientY; st = Date.now(); ok = true;
  }, {passive: true});
  document.addEventListener('touchend', e => {
    if (!ok) return; ok = false;
    const t = e.changedTouches[0], dx = t.clientX - sx, dy = t.clientY - sy;
    if (Date.now() - st > 700 || Math.abs(dx) < 70 || Math.abs(dy) > Math.abs(dx) * 0.5) return;
    if (String(getSelection()).length) return;
    const ord = [...document.querySelectorAll('.tab')].map(b => b.dataset.tab), a = document.querySelector('.tab.active');
    const i = a ? ord.indexOf(a.dataset.tab) : -1; if (i < 0) return;
    const d = dx < 0 ? -1 : 1, n = ord.length, j = (i - d + n) % n;   // دايرة: بعد آخر تاب يرجع للأول
    if (n > 1) showTab(ord[j], d);
  }, {passive: true});
})();

/* ---------- commands ---------- */
let cmds = [], editId = 0, irList = [], learnT = null;
const PH = {http: 'http://192.168.1.36/TOGGLE', wol: 'D8:5E:D3:C2:A1:F8', relay: '1:on  |  1:off  |  2:toggle', delay: '5'};

function fillIrSel(sel, val) {
  const cur = val !== undefined ? String(val) : sel.value;
  sel.textContent = '';
  irList.forEach(i => { const o = el('option', '', i.name); o.value = i.id; sel.appendChild(o); });
  if (!irList.length) { const o = el('option', '', 'لا توجد إشارات - تعلّم من تبويب IR'); o.value = ''; sel.appendChild(o); }
  sel.value = cur;
}
function refreshIrSelects() { document.querySelectorAll('.irsel').forEach(s => fillIrSel(s)); }

function addStep(type, val) {
  const row = el('div', 'step');
  const sel = el('select', 'tsel');
  [['http', 'HTTP'], ['wol', 'WOL'], ['ir', 'IR'], ['strip', 'مشترك'], ['delay', 'تأخير'], ['relay', 'RELAY']].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sel.appendChild(o); });
  sel.value = type || 'http';
  const inp = el('input', 'tin'); inp.type = 'text'; inp.dir = 'ltr';
  const irs = el('select', 'irsel'); fillIrSel(irs, type === 'ir' ? val : undefined);
  // مشترك: مخرج + حالة
  const sv = el('div', 'sv');
  const sn = el('select', 'sn'); [['0', 'كل المخارج'], ['1', outName(0)], ['2', outName(1)], ['3', outName(2)], ['4', outName(3)]].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sn.appendChild(o); });
  const sa = el('select', 'sa'); [['on', 'تشغيل'], ['off', 'إطفاء'], ['toggle', 'تبديل']].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sa.appendChild(o); });
  sv.appendChild(sn); sv.appendChild(sa);
  // تأخير: ثواني
  const dl = el('div', 'dl');
  const din = el('input', 'din'); din.type = 'number'; din.min = '0.1'; din.max = '3600'; din.step = '0.5'; din.dir = 'ltr'; din.placeholder = '5';
  dl.appendChild(din); dl.appendChild(el('span', '', 'ثانية'));
  if (type === 'strip' && val) { const [n, a] = String(val).split(':'); sn.value = n; sa.value = a || 'on'; }
  else if (type === 'delay') din.value = val || '';
  else if (type !== 'ir') inp.value = val || '';
  const upd = () => {
    const t = sel.value;
    inp.hidden = !(t === 'http' || t === 'wol' || t === 'relay');
    irs.hidden = t !== 'ir'; sv.hidden = t !== 'strip'; dl.hidden = t !== 'delay';
    if (!inp.hidden) inp.placeholder = PH[t];
  };
  sel.onchange = upd; upd();
  const rm = el('button', 'btn del', '✕');
  rm.onclick = () => { if ($('#steps').children.length > 1) row.remove(); };
  const grip = el('span', 'grip', '⠿'); grip.title = 'اسحب للترتيب';
  const mv = el('div', 'mv');
  const up = el('button', 'btn ghost', '▲'); up.onclick = () => { const p = row.previousElementSibling; if (p) row.parentNode.insertBefore(row, p); };
  const dn = el('button', 'btn ghost', '▼'); dn.onclick = () => { const n = row.nextElementSibling; if (n) row.parentNode.insertBefore(n, row); };
  mv.appendChild(up); mv.appendChild(dn);
  [grip, mv, sel, inp, irs, sv, dl, rm].forEach(x => row.appendChild(x));
  setupDrag(row, grip);
  $('#steps').appendChild(row);
}
/* سحب وإفلات الخطوات (ماوس + لمس) */
let dragRow = null;
function setupDrag(row, grip) {
  grip.onpointerdown = e => {
    e.preventDefault(); dragRow = row; row.classList.add('dragging');
    grip.setPointerCapture(e.pointerId);
  };
  grip.onpointermove = e => {
    if (dragRow !== row) return;
    const t = document.elementFromPoint(e.clientX, e.clientY);
    const over = t && t.closest ? t.closest('#steps .step') : null;
    document.querySelectorAll('#steps .step.over').forEach(x => x.classList.remove('over'));
    if (over && over !== row) over.classList.add('over');
  };
  const end = e => {
    if (dragRow !== row) return;
    const t = document.elementFromPoint(e.clientX, e.clientY);
    const over = t && t.closest ? t.closest('#steps .step') : null;
    if (over && over !== row) {
      const rows = [...$('#steps').children];
      if (rows.indexOf(row) < rows.indexOf(over)) over.after(row); else over.before(row);
    }
    document.querySelectorAll('#steps .step').forEach(x => x.classList.remove('over', 'dragging'));
    dragRow = null;
  };
  grip.onpointerup = end; grip.onpointercancel = end;
}
function stepVal(r) {
  const t = r.querySelector('.tsel').value;
  if (t === 'ir') return r.querySelector('.irsel').value;
  if (t === 'strip') return r.querySelector('.sn').value + ':' + r.querySelector('.sa').value;
  if (t === 'delay') return r.querySelector('.din').value;
  return r.querySelector('.tin').value;
}
function stepLabel(s) {
  if (s.type === 'ir') return 'IR: ' + ((irList.find(i => String(i.id) === String(s.val)) || {}).name || '؟');
  if (s.type === 'delay') return '⏱ ' + s.val + ' ث';
  if (s.type === 'strip') { const [n, a] = String(s.val).split(':'); return 'مشترك ' + (n === '0' ? 'الكل' : outName(+n - 1)) + ': ' + ({on: 'تشغيل', off: 'إطفاء', toggle: 'تبديل'}[a] || a); }
  return s.val;
}

function resetForm() {
  editId = 0;
  $('#name').value = ''; $('#cat').value = ''; $('#phrases').value = '';
  $('#steps').textContent = ''; addStep();
  $('#form-title').textContent = 'إضافة أمر جديد';
  $('#cancelBtn').hidden = true;
}

function editCmd(id) {
  const c = cmds.find(x => x.id === id); if (!c) return;
  editId = id; openForm();
  $('#name').value = c.name; $('#cat').value = c.cat || '';
  $('#phrases').value = (c.phrases || []).join('\n');
  $('#steps').textContent = '';
  (c.steps || []).forEach(s => addStep(s.type, s.val));
  if (!$('#steps').children.length) addStep();
  $('#form-title').textContent = 'تعديل: ' + c.name;
  $('#cancelBtn').hidden = false;
  window.scrollTo({top: 0, behavior: 'smooth'});
}

async function loadCommands() {
  try {
    const r = await (await fetch('/api/commands', {cache: 'no-store'})).json();
    cmds = Array.isArray(r) ? r : [];
    renderCommands(); renderIr();
  } catch (e) { toast('تعذر تحميل الأوامر', true); }
}

function renderCommands() {
  const box = $('#groups'); box.textContent = '';
  $('#cmd-count').textContent = cmds.length;
  const found = [...new Set(cmds.map(c => c.cat || 'عام'))];
  const saved = lsGet('catOrder', []);
  const cats = [...saved.filter(c => found.includes(c)), ...found.filter(c => !saved.includes(c))];
  const openCats = lsGet('catOpen', []);
  const q = ($('#q').value || '').trim().toLowerCase();
  const dl = $('#cats'); dl.textContent = '';
  cats.forEach(c => { const o = el('option'); o.value = c; dl.appendChild(o); });
  if (!cmds.length) {
    const card = el('div', 'card'); card.appendChild(el('div', 'empty', 'لا توجد أوامر محفوظة بعد')); box.appendChild(card); return;
  }
  cats.forEach((cat, idx) => {
    let list = cmds.filter(c => (c.cat || 'عام') === cat);
    if (q) { list = list.filter(c => (cat + ' ' + c.name + ' ' + (c.phrases || []).join(' ')).toLowerCase().includes(q)); if (!list.length) return; }
    const card = el('div', 'card');
    const open = !!q || openCats.includes(cat);
    const h = el('h2', 'cath');
    const left = el('span', 'catt');
    const arrow = el('span', 'carr', open ? '▾' : '◂');
    left.appendChild(arrow); left.appendChild(el('span', '', cat)); left.appendChild(el('span', 'badge', list.length));
    const mvb = el('span', 'cmv');
    const up = el('button', 'btn ghost', '▲'); up.title = 'تحريك لفوق'; up.disabled = idx === 0 || !!q;
    const dn = el('button', 'btn ghost', '▼'); dn.title = 'تحريك لتحت'; dn.disabled = idx === cats.length - 1 || !!q;
    up.onclick = e => { e.stopPropagation(); moveCat(cats, idx, -1); };
    dn.onclick = e => { e.stopPropagation(); moveCat(cats, idx, 1); };
    mvb.appendChild(up); mvb.appendChild(dn);
    h.appendChild(left); h.appendChild(mvb);
    card.appendChild(h);
    const body = el('div', 'catbody'); body.hidden = !open; h.style.marginBottom = open ? '' : '0';
    h.onclick = () => {
      if (q) return;
      const o = lsGet('catOpen', []), k = o.indexOf(cat), now = k < 0;
      if (now) o.push(cat); else o.splice(k, 1);
      lsSet('catOpen', o);
      body.hidden = !now; arrow.textContent = now ? '▾' : '◂'; h.style.marginBottom = now ? '' : '0';
    };
    card.appendChild(body);
    list.forEach(c => {
      const row = el('div', 'cmd');
      const main = el('div', 'cmd-main');
      main.appendChild(el('div', 'cmd-name', c.name));
      const meta = el('div', 'cmd-meta');
      meta.appendChild(el('span', 'badge', (c.steps || []).length + ' خطوة'));
      meta.appendChild(el('span', 'badge', (c.phrases || []).length + ' جملة'));
      main.appendChild(meta);
      const ph = el('div', 'phrases');
      (c.phrases || []).forEach(p => ph.appendChild(el('span', 'ph', p)));
      main.appendChild(ph);
      const stl = el('div', 'stl');
      (c.steps || []).forEach((st, k) => {
        const TY = {http: 'HTTP', wol: 'WOL', ir: 'IR', strip: 'مشترك', delay: 'تأخير', relay: 'RELAY'};
        let v = String(st.val);
        if (st.type === 'ir') v = (irList.find(i => String(i.id) === v) || {}).name || '؟';
        else if (st.type === 'delay') v = v + ' ثانية';
        else if (st.type === 'strip') { const [n, a] = v.split(':'); v = (n === '0' ? 'كل المخارج' : outName(+n - 1)) + ' ← ' + ({on: 'تشغيل', off: 'إطفاء', toggle: 'تبديل'}[a] || a); }
        const r = el('div', 'stp');
        const vv = el('span', 'v', v); vv.dir = 'auto';
        r.appendChild(el('span', 'n', k + 1)); r.appendChild(el('span', 'ty', TY[st.type] || st.type)); r.appendChild(vv);
        stl.appendChild(r);
      });
      main.appendChild(stl);
      const lk = el('div', 'cmd-link');
      const url = location.origin + '/api/run?id=' + c.id;
      const inp = el('input'); inp.type = 'text'; inp.readOnly = true; inp.value = url;
      inp.onfocus = () => inp.select();
      const cp = el('button', 'btn ghost', '📋 نسخ'); cp.onclick = () => copyText(url);
      lk.appendChild(inp); lk.appendChild(cp);
      main.appendChild(lk);
      const acts = el('div', 'cmd-actions');
      const run = el('button', 'btn run', '▶'); run.onclick = () => runCmd(c.id, c.name, run);
      const ed = el('button', 'btn ghost', '✎'); ed.onclick = () => editCmd(c.id);
      const del = el('button', 'btn del', '🗑'); del.onclick = () => delCmd(c.id, c.name);
      acts.appendChild(run); acts.appendChild(ed); acts.appendChild(del);
      row.appendChild(main); row.appendChild(acts);
      body.appendChild(row);
    });
    box.appendChild(card);
  });
  if (q && !box.children.length) { const card = el('div', 'card'); card.appendChild(el('div', 'empty', 'لا نتائج للبحث')); box.appendChild(card); }
}
function setAllCats(open) {
  const f = [...new Set(cmds.map(c => c.cat || 'عام'))];
  lsSet('catOpen', open ? f : []); renderCommands();
}
function moveCat(cats, i, d) {
  const j = i + d; if (j < 0 || j >= cats.length) return;
  [cats[i], cats[j]] = [cats[j], cats[i]];
  lsSet('catOrder', cats); renderCommands();
}

async function saveCommand() {
  const name = $('#name').value.trim();
  const cat = $('#cat').value.trim() || 'عام';
  let phrases = $('#phrases').value.split('\n').map(s => s.trim().toLowerCase()).filter(Boolean);
  if (!phrases.length && name) phrases = [name.toLowerCase()];
  const steps = [...$('#steps').children].map(r => ({type: r.querySelector('.tsel').value, val: String(stepVal(r)).trim()})).filter(s => s.val);
  if (!name || !phrases.length || !steps.length) { toast('أكمل البيانات المطلوبة', true); return; }
  try {
    const r = await fetch('/api/save', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({id: editId, name, cat, phrases, steps})});
    if (!r.ok) throw 0;
    toast(editId ? 'تم تحديث الأمر' : 'تم حفظ الأمر');
    resetForm(); loadCommands();
  } catch (e) { toast('فشل الحفظ (المساحة ممتلئة؟)', true); }
}

async function runCmd(id, name, btn) {
  const old = btn.textContent; btn.disabled = true; btn.textContent = '...';
  try {
    const r = await post('/api/run', {id});
    toast(r.ok ? 'تم إرسال: ' + name : 'فشل التنفيذ: ' + name, !r.ok);
  } catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
  btn.disabled = false; btn.textContent = old;
}

async function delCmd(id, name) {
  if (!confirm('حذف الأمر "' + name + '"؟')) return;
  try {
    const r = await post('/api/delete', {id});
    if (!r.ok) throw 0;
    if (editId === id) resetForm();
    toast('تم الحذف'); loadCommands();
  } catch (e) { toast('فشل الحذف', true); }
}

async function clearAllCommands() {
  if (!confirm('هل أنت متأكد من مسح جميع الأوامر؟')) return;
  await post('/api/clear'); resetForm(); toast('تم مسح كل الأوامر'); loadCommands();
}

/* ---------- ما سمعه الجهاز + الميكروفون ---------- */
async function loadHeard() {
  try {
    const a = await (await fetch('/api/heard', {cache: 'no-store'})).json();
    const box = $('#heard'); box.textContent = '';
    if (!a.length) { box.appendChild(el('div', 'empty', $('#heardOn').checked ? 'لسه ما سمعش حاجة' : 'التسجيل متوقف')); return; }
    a.forEach(h => {
      const row = el('div', 'cmd hrow');
      const main = el('div', 'cmd-main'); main.appendChild(el('div', 'cmd-name', (h.ok ? '✔ ' : '✖ ') + h.t));
      const acts = el('div', 'cmd-actions');
      const add = el('button', 'btn ghost', '＋');
      add.onclick = () => { openForm(); const ta = $('#phrases'); ta.value = (ta.value ? ta.value.replace(/\n*$/, '') + '\n' : '') + h.t; toast('اتضافت لخانة الجمل'); window.scrollTo({top: 0, behavior: 'smooth'}); };
      acts.appendChild(add); row.appendChild(main); row.appendChild(acts); box.appendChild(row);
    });
  } catch (e) {}
}
function toggleHeard(force) {
  const open = force !== undefined ? force : $('#heardBody').hidden;
  $('#heardBody').hidden = !open;
  $('#heardArrow').textContent = open ? '▾' : '◂';
  $('#heardHead').style.marginBottom = open ? '' : '0';
  try { localStorage.setItem('heardOpen', open ? '1' : '0'); } catch (e) {}
  if (open) loadHeardCfg().then(loadHeard);
}
async function clearHeard() { try { await post('/api/heard/clear'); toast('تم المسح'); loadHeard(); } catch (e) { toast('لا يوجد اتصال بالجهاز', true); } }
async function setHeardOn() {
  const on = $('#heardOn').checked;
  try { await post('/api/heard/cfg', {on: on ? 1 : 0}); toast(on ? 'التسجيل شغّال' : 'التسجيل متوقف'); loadHeard(); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function loadHeardCfg() { try { const c = await (await fetch('/api/heard/cfg', {cache: 'no-store'})).json(); $('#heardOn').checked = !!c.on; } catch (e) {} }
async function loadMic() {
  try {
    const m = await (await fetch('/api/mic', {cache: 'no-store'})).json();
    if (document.activeElement !== $('#micg')) $('#micg').value = m.g;
    if (document.activeElement !== $('#micv')) $('#micv').value = m.v;
    $('#micinfo').textContent = 'ضوضاء الغرفة: ' + m.n + ' | آخر تسجيل: قمة ' + m.pk + ' · تضخيم ' + m.gain + 'x · كلام ' + m.ms + ' ms';
  } catch (e) {}
}
async function saveMic() {
  try { const r = await post('/api/mic', {g: $('#micg').value, v: $('#micv').value}); toast(r.ok ? 'تم الحفظ' : 'قيمة غير صالحة', !r.ok); loadMic(); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
setInterval(() => { if (!$('#sub-mic').hidden && !$('#tab-maint').hidden) loadMic(); if (!$('#tab-cmds').hidden && !$('#heardBody').hidden) loadHeard(); }, 2500);

/* ---------- المشترك ---------- */
let learnEnd = 0;
let stripT = null;
const outNames = () => lsGet('outNames', {});
const outName = i => outNames()[i] || ('مخرج ' + (i + 1));
function renameOut(i) {
  const v = prompt('اسم المخرج ' + (i + 1) + ':', outName(i)); if (v === null) return;
  const o = outNames(); if (v.trim()) o[i] = v.trim(); else delete o[i];
  lsSet('outNames', o); stripStatus();
}
function startStrip() { stripStatus(); clearInterval(stripT); stripT = setInterval(stripStatus, 2000); }
function stopStrip() { clearInterval(stripT); stripT = null; }
const PLUG = '<svg class="pg" viewBox="0 0 52 52"><circle cx="26" cy="26" r="23" fill="none" stroke="currentColor" stroke-width="3"/><rect x="17" y="19" width="5" height="12" rx="2.5" fill="currentColor"/><rect x="30" y="19" width="5" height="12" rx="2.5" fill="currentColor"/></svg>';
let stripBuilt = false, stripLast = null;
function buildStrip() {
  const g = $('#st-grid'); g.textContent = '';
  for (let i = 0; i < 4; i++) {
    const t = el('div', 'tile dis'); t.id = 'so' + i;
    t.innerHTML = PLUG + '<div class="nm"></div><div class="w"></div><div class="a"></div><div class="sub"></div><span class="wn" hidden>⚠</span>';
    const ed = el('button', 'ed', '✎'); ed.title = 'تغيير الاسم'; ed.onclick = e => { e.stopPropagation(); renameOut(i); };
    t.appendChild(ed);
    t.onclick = () => { if (!t.classList.contains('dis')) stripSet(i + 1, t.classList.contains('on') ? 'off' : 'on'); };
    g.appendChild(t);
  }
  stripBuilt = true;
}
function stripHub() { if (!stripLast || !stripLast.on) return; stripSet(0, stripLast.o.some(o => o.on) ? 'off' : 'on'); }
async function stripStatus() {
  try {
    const s = await (await fetch('/api/strip/status', {cache: 'no-store'})).json();
    if (!stripBuilt) buildStrip();
    stripLast = s;
    $('#st-ip').textContent = s.ip;
    const stale = s.on && s.age > 30;
    $('#st-live').className = 'lv' + (s.on && !stale ? '' : ' off'); $('#st-livetx').textContent = s.on ? (stale ? 'STALE' : 'LIVE') : 'OFFLINE';
    $('#st-badge').textContent = s.on ? 'متصل' : 'غير متصل'; $('#st-badge').className = 'pill' + (s.on ? '' : ' off');
    $('#st-card').classList.toggle('on', !!s.on);
    $('#st-info').textContent = s.on ? ('MAC ' + s.mac + ' | FW ' + s.fw + ' | آخر قراءة: ' + (s.age < 0 ? '--' : s.age + ' ث')) : 'مستني اتصال المشترك على ' + s.ip + ':' + s.port;
    const V = s.v > 0 ? s.v : 220, ap = s.v > 0 ? '' : '≈', amp = w => ap + (w / V).toFixed(2) + ' A';
    const tw = s.o.reduce((a, o) => a + o.w, 0), twh = s.o.reduce((a, o) => a + o.wh, 0), cnt = s.o.filter(o => o.on).length;
    $('#st-w').textContent = tw.toFixed(1); $('#st-a').textContent = amp(tw);
    $('#st-lbl').textContent = 'الاستهلاك الآن' + (stale ? ' ⚠ القراءات قديمة' : '');
    $('#st-cnt').textContent = cnt + '/4'; $('#st-kwh').textContent = (twh / 1000).toFixed(3); $('#st-age').textContent = s.on && s.age >= 0 ? s.age + ' ث' : '--';
    $('#st-sum').textContent = tw.toFixed(1) + ' W · ' + (twh / 1000).toFixed(3) + ' kWh · ' + amp(tw) + (s.v > 0 ? ' · ' + s.v.toFixed(0) + ' V' : ' (بافتراض 220V)');
    const hub = $('#st-hub'); hub.disabled = !s.on; hub.classList.toggle('on', cnt > 0);
    s.o.forEach((o, i) => {
      const t = $('#so' + i);
      t.classList.toggle('on', !!o.on); t.classList.toggle('dis', !s.on); t.classList.toggle('bad', !o.ok);
      t.querySelector('.nm').textContent = outName(i);
      t.querySelector('.w').textContent = o.w.toFixed(1) + ' W'; t.querySelector('.a').textContent = amp(o.w);
      t.querySelector('.sub').textContent = (o.wh / 1000).toFixed(3) + ' kWh · ' + o.t + '°C';
      t.querySelector('.wn').hidden = !!o.ok;
    });
  } catch (e) {}
}
async function stripSet(n, st) {
  try { const r = await post('/api/strip/set', {n, s: st}); toast(r.ok ? 'تم الإرسال' : (r.status === 409 ? 'المشترك غير متصل' : 'فشل'), !r.ok); setTimeout(stripStatus, 900); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function stripRefresh(manual) {
  try { const r = await post('/api/strip/refresh'); if (manual) toast(r.ok ? 'جاري التحديث...' : 'المشترك غير متصل', !r.ok); setTimeout(stripStatus, 700); }
  catch (e) { if (manual) toast('لا يوجد اتصال بالجهاز', true); }
}
async function stripRaw() {
  const pre = $('#st-raw');
  try {
    const t = (await (await fetch('/api/strip/raw', {cache: 'no-store'})).text()).trim();
    const p = t.split(':'); let out = t || 'لا يوجد رد بعد';
    if (p.length === 8) out = [0, 1, 2, 3].map(k => 'outlet ' + p[k * 2] + ' → ' + p[k * 2 + 1].split(';').map((v, i) => 'F' + i + '=' + v).join(' ')).join('\n');
    pre.textContent = out; pre.hidden = false;
  } catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}

/* ---------- IR ---------- */
const IR_PINS = [1,2,4,6,9,10,11,12,13,14,18,21,38,39,40,41,42,47];
function fillPins() {
  ['#irtx', '#irrx'].forEach(s => { const sel = $(s); sel.textContent = ''; IR_PINS.forEach(p => { const o = el('option', '', 'GPIO ' + p); o.value = p; sel.appendChild(o); }); });
}
async function loadIrCfg() {
  try { const c = await (await fetch('/api/ir/cfg', {cache: 'no-store'})).json(); $('#irtx').value = c.tx; $('#irrx').value = c.rx; } catch (e) {}
}
async function saveIrCfg() {
  const tx = $('#irtx').value, rx = $('#irrx').value;
  if (tx === rx) { toast('لازم الطرفين يكونوا مختلفين', true); return; }
  try { const r = await post('/api/ir/cfg', {tx, rx}); if (!r.ok) throw 0; toast('تم حفظ الأطراف'); }
  catch (e) { toast('فشل حفظ الأطراف', true); }
}
async function loadIrList() {
  try { const r = await (await fetch('/api/ir/list', {cache: 'no-store'})).json(); irList = Array.isArray(r) ? r : []; } catch (e) { irList = []; }
  renderIr(); refreshIrSelects(); if (cmds.length) renderCommands();
}
function renderIr() {
  const box = $('#irlist'); box.textContent = '';
  $('#ir-count').textContent = irList.length;
  const iq = $('#irq'); iq.hidden = irList.length < 6;
  const qq = iq.hidden ? '' : iq.value.trim().toLowerCase();
  if (!irList.length) { box.appendChild(el('div', 'empty', 'لا توجد إشارات محفوظة بعد')); return; }
  const shown = irList.filter(c => !qq || c.name.toLowerCase().includes(qq));
  if (!shown.length) { box.appendChild(el('div', 'empty', 'لا نتائج')); return; }
  shown.forEach(c => {
    const row = el('div', 'cmd');
    const main = el('div', 'cmd-main');
    main.appendChild(el('div', 'cmd-name', c.name));
    const meta = el('div', 'cmd-meta');
    meta.appendChild(el('span', 'badge', c.len + ' نبضة'));
    meta.appendChild(el('code', '', c.khz + ' kHz × ' + c.rep));
    const uc = cmds.filter(x => (x.steps || []).some(s => s.type === 'ir' && String(s.val) === String(c.id))).length;
    if (uc) meta.appendChild(el('span', 'badge', uc + ' أمر'));
    main.appendChild(meta);
    const acts = el('div', 'cmd-actions');
    const run = el('button', 'btn run', '▶'); run.onclick = () => sendIr(c.id, run);
    const ed = el('button', 'btn ghost', '✎'); ed.onclick = () => renameIr(c);
    const del = el('button', 'btn del', '🗑'); del.onclick = () => delIr(c);
    acts.appendChild(run); acts.appendChild(ed); acts.appendChild(del);
    row.appendChild(main); row.appendChild(acts); box.appendChild(row);
  });
}
async function startLearn() {
  try { await post('/api/ir/learn'); } catch (e) { toast('لا يوجد اتصال بالجهاز', true); return; }
  $('#irnew').hidden = true; learnEnd = Date.now() + 15000; $('#learnBtn').disabled = true;
  $('#irstat').textContent = 'وجّه الريموت ناحية المستقبل واضغط الزر... (15 ث)';
  clearInterval(learnT); learnT = setInterval(pollLearn, 500);
}
async function pollLearn() {
  try {
    const s = await (await fetch('/api/ir/status', {cache: 'no-store'})).json();
    if (s.state === 'captured') { clearInterval(learnT); $('#learnBtn').disabled = false; $('#irstat').textContent = 'تم الالتقاط: ' + s.len + ' نبضة'; $('#irnew').hidden = false; }
    else if (s.state === 'timeout') { clearInterval(learnT); $('#learnBtn').disabled = false; $('#irstat').textContent = 'انتهى الوقت - جرّب تاني'; }
    else if (s.state === 'idle') { clearInterval(learnT); $('#learnBtn').disabled = false; $('#irstat').textContent = ''; }
    else $('#irstat').textContent = 'وجّه الريموت ناحية المستقبل واضغط الزر... (' + Math.max(0, Math.ceil((learnEnd - Date.now()) / 1000)) + ' ث)';
  } catch (e) {}
}
async function testIr() {
  try { const r = await post('/api/ir/send', {pending: 1, khz: $('#irkhz').value, rep: $('#irrep').value}); toast(r.ok ? 'تم الإرسال' : 'فشل الإرسال', !r.ok); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function saveIr() {
  const name = $('#irname').value.trim();
  if (!name) { toast('اكتب اسم للإشارة', true); return; }
  try {
    const r = await post('/api/ir/save', {id: 0, name, khz: $('#irkhz').value, rep: $('#irrep').value});
    if (!r.ok) throw 0;
    $('#irname').value = ''; $('#irnew').hidden = true; $('#irstat').textContent = '';
    toast('تم حفظ الإشارة'); loadIrList();
  } catch (e) { toast('فشل الحفظ', true); }
}
async function sendIr(id, btn) {
  const old = btn.textContent; btn.disabled = true; btn.textContent = '...';
  try { const r = await post('/api/ir/send', {id}); toast(r.ok ? 'تم الإرسال' : 'فشل الإرسال', !r.ok); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
  btn.disabled = false; btn.textContent = old;
}
async function renameIr(c) {
  const name = prompt('اسم الإشارة:', c.name); if (!name || !name.trim()) return;
  try { const r = await post('/api/ir/save', {id: c.id, name: name.trim(), khz: c.khz, rep: c.rep}); if (!r.ok) throw 0; toast('تم التعديل'); loadIrList(); }
  catch (e) { toast('فشل التعديل', true); }
}
async function delIr(c) {
  const used = cmds.some(x => (x.steps || []).some(s => s.type === 'ir' && String(s.val) === String(c.id)));
  if (!confirm('حذف الإشارة "' + c.name + '"؟' + (used ? '\nمستخدمة في أوامر صوتية!' : ''))) return;
  try { const r = await post('/api/ir/delete', {id: c.id}); if (!r.ok) throw 0; toast('تم الحذف'); loadIrList(); }
  catch (e) { toast('فشل الحذف', true); }
}

/* ---------- maintenance: info ---------- */
function fmtUp(s) {
  const d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600), m = Math.floor(s % 3600 / 60);
  return (d ? d + 'd ' : '') + h + 'h ' + m + 'm';
}
let lastTimePush = 0;
async function loadInfo() {
  try {
    const i = await (await fetch('/api/info', {cache:'no-store'})).json();
    if (i.sync === 0 && Date.now() - lastTimePush > 30000) { lastTimePush = Date.now(); post('/api/time', {ts: Math.floor(Date.now() / 1000)}).catch(() => {}); }   // NTP مش شغال: الموبايل يضبط الساعة
    $('#ip').textContent = i.ip; $('#dot').className = 'dot on';
    const box = $('#info'); box.textContent = '';
    [['IP', i.ip], ['Wi-Fi', i.rssi + ' dBm · ' + (i.rssi > -60 ? 'Excellent' : i.rssi > -70 ? 'Good' : i.rssi > -80 ? 'Fair' : 'Weak')], ['RAM', Math.round(i.heap / 1024) + ' KB'],
     ['PSRAM', Math.round(i.psram / 1024 / 1024 * 10) / 10 + ' MB'], ['Uptime', fmtUp(i.up)], ['Wake word', i.wake], ['Time', (i.sync ? '' : '⚠ ') + (i.now ? i.now.slice(11) : 'غير مضبوط')]]
    .forEach(([a, b]) => { const c = el('div', 'chip'); c.appendChild(el('small', '', a)); c.appendChild(el('b', '', b)); box.appendChild(c); });
  } catch (e) { $('#dot').className = 'dot'; }
}

/* ---------- maintenance: serial monitor ---------- */
let since = 0, carry = '', logTimer = null, infoTimer = null, paused = false, busy = false;
const logLines = [];
function cls(t) {
  if (/error|fail|not found|unable|not recognized/i.test(t)) return 'err';
  if (/wake word detected/i.test(t)) return 'wake';
  if (/executed|sent wol|transcribed|connected|loaded|saved|manual/i.test(t)) return 'ok';
  if (/recording|processing/i.test(t)) return 'proc';
  return '';
}
function addLine(text, c) {
  const term = $('#term');
  let time = null; const m = /^\[([+\d:. APM]+)\] /.exec(text);       // وقت الجهاز نفسه (من الفيرموير)
  if (m) { time = m[1]; text = text.slice(m[0].length); }
  if (!time) time = new Date().toLocaleTimeString('en-US', {hour12: true});
  const d = el('div', c || cls(text));
  d.appendChild(el('span', 't', time)); d.appendChild(document.createTextNode(text));
  term.appendChild(d); d.hidden = !logMatch(d);
  logLines.push(time + ' ' + text);
  while (term.childNodes.length > 500) term.removeChild(term.firstChild);
  if (logLines.length > 2000) logLines.splice(0, 500);
  if ($('#autoscroll').checked) term.scrollTop = term.scrollHeight;
}
function logMatch(d) {
  const lv = $('#logLvl').value, q = $('#logq').value.trim().toLowerCase();
  return (!lv || d.classList.contains(lv)) && (!q || d.textContent.toLowerCase().includes(q));
}
function applyLogFilter() { [...$('#term').children].forEach(d => d.hidden = !logMatch(d)); }
function pushText(txt) {
  const parts = (carry + txt).split('\n');
  carry = parts.pop();
  parts.forEach(p => { p = p.replace(/\r/g, ''); if (p.trim()) addLine(p); });
}
async function pollLog() {
  if (paused || busy) return;
  busy = true;
  try {
    const r = await fetch('/api/log?since=' + since, {cache:'no-store'});
    const next = parseInt(r.headers.get('X-Next') || '0', 10);
    const txt = await r.text();
    if (next < since) { carry = ''; addLine('— الجهاز اتعمله إعادة تشغيل —', 'sys'); }
    since = next;
    if (txt) pushText(txt);
    $('#dot').className = 'dot on';
  } catch (e) { $('#dot').className = 'dot'; }
  busy = false;
}
function startLog() {
  stopLog(); pollLog();
  logTimer = setInterval(pollLog, 1000);
}
function stopLog() { clearInterval(logTimer); clearInterval(infoTimer); }
function togglePause() {
  paused = !paused;
  $('#pauseBtn').textContent = paused ? '▶ استئناف' : '⏸ إيقاف';
}
function clearTerm() { $('#term').textContent = ''; logLines.length = 0; post('/api/log/clear').catch(() => {}); }
async function copyText(t) {
  try { await navigator.clipboard.writeText(t); toast('تم نسخ الرابط'); return; } catch (e) {}
  const ta = el('textarea'); ta.value = t; ta.style.position = 'fixed'; ta.style.opacity = '0';
  document.body.appendChild(ta); ta.focus(); ta.select();
  let ok = false; try { ok = document.execCommand('copy'); } catch (e) {}
  ta.remove(); toast(ok ? 'تم نسخ الرابط' : 'انسخ الرابط يدويًا', !ok);
}
async function copyLog() {
  try { await navigator.clipboard.writeText(logLines.join('\n')); toast('تم النسخ'); }
  catch (e) {
    const ta = el('textarea'); ta.value = logLines.join('\n'); document.body.appendChild(ta);
    ta.select(); document.execCommand('copy'); ta.remove(); toast('تم النسخ');
  }
}
function downloadLog() {
  const a = el('a'); a.href = URL.createObjectURL(new Blob([logLines.join('\n')], {type:'text/plain'}));
  a.download = 'serial-log.txt'; a.click(); URL.revokeObjectURL(a.href);
}
async function sendSay() {
  const t = $('#say').value.trim(); if (!t) return;
  try { await post('/api/say', {text: t}); $('#say').value = ''; addLine('> ' + t, 'sys'); setTimeout(pollLog, 400); }
  catch (e) { toast('فشل الإرسال', true); }
}
async function reboot() {
  if (!confirm('إعادة تشغيل الجهاز؟')) return;
  try { await post('/api/reboot'); toast('جاري إعادة التشغيل...'); } catch (e) {}
}

/* ---------- نسخة احتياطية: تصدير / استيراد ---------- */
async function exportCmds() {
  try {
    const r = await (await fetch('/api/commands', {cache: 'no-store'})).json();
    let il = []; try { const x = await (await fetch('/api/ir/list', {cache: 'no-store'})).json(); il = Array.isArray(x) ? x : []; } catch (e) {}
    const ir = [];
    for (const i of il) {
      const t = await fetch('/api/ir/raw?id=' + i.id, {cache: 'no-store'});
      if (!t.ok) { toast('فشل قراءة الإشارة: ' + i.name, true); return; }
      ir.push({id: i.id, name: i.name, khz: i.khz, rep: i.rep, pulses: (await t.text()).trim().split(',').map(Number)});
    }
    const data = {app: 'voice-hub', version: 2, exported: new Date().toISOString(), commands: Array.isArray(r) ? r : [], ir};
    const a = el('a'); a.href = URL.createObjectURL(new Blob([JSON.stringify(data, null, 2)], {type: 'application/json'}));
    a.download = 'voice-hub-backup-' + new Date().toISOString().slice(0, 10) + '.json';
    document.body.appendChild(a); a.click(); a.remove(); setTimeout(() => URL.revokeObjectURL(a.href), 1000);
    toast('تم تصدير ' + data.commands.length + ' أمر و ' + ir.length + ' إشارة IR');
  } catch (e) { toast('فشل التصدير', true); }
}
async function importCmds(inp) {
  const f = inp.files[0]; inp.value = ''; if (!f) return;
  let list, irs;
  try {
    const d = JSON.parse(await f.text());
    list = Array.isArray(d) ? d : d.commands;
    if (!Array.isArray(list)) throw 0;
    irs = Array.isArray(d.ir) ? d.ir.filter(x => x && x.name && Array.isArray(x.pulses) && x.pulses.length >= 5) : [];
  } catch (e) { toast('الملف مش صالح', true); return; }
  list = list.filter(c => c && c.name && Array.isArray(c.phrases) && Array.isArray(c.steps));
  if (!list.length && !irs.length) { toast('مفيش بيانات صالحة في الملف', true); return; }
  const replace = confirm('الملف فيه ' + list.length + ' أمر و ' + irs.length + ' إشارة IR.\n\nOK = استبدال كل الموجود حالياً\nCancel = إضافتها على الموجود');
  if (!replace && !confirm('إضافة ' + list.length + ' أمر و ' + irs.length + ' إشارة على الموجود؟')) return;
  try {
    if (replace) {
      await post('/api/clear');
      if (irs.length) {
        const cur = await (await fetch('/api/ir/list', {cache: 'no-store'})).json();
        for (const c of cur) await post('/api/ir/delete', {id: c.id});
      }
    }
    // الإشارات الأول، وبنسجل الرقم الجديد لكل إشارة عشان نعدّل الأوامر عليه
    const map = {}; let irOk = 0;
    for (const i of irs) {
      const q = new URLSearchParams({name: i.name, khz: i.khz || 38, rep: i.rep || 1}).toString();
      const r = await fetch('/api/ir/import?' + q, {method: 'POST', headers: {'Content-Type': 'text/plain'}, body: i.pulses.join(',')});
      if (r.ok) { map[String(i.id)] = (await r.text()).trim(); irOk++; }
    }
    let ok = 0;
    for (const c of list) {
      const steps = c.steps.map(s => (s.type === 'ir' && irs.length) ? {type: 'ir', val: map[String(s.val)] || String(s.val)} : s);
      const r = await fetch('/api/save', {method: 'POST', headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({id: 0, name: c.name, cat: c.cat || 'عام', phrases: c.phrases, steps})});
      if (r.ok) ok++; else break;
    }
    const bad = ok !== list.length || irOk !== irs.length;
    toast('تم استيراد ' + ok + '/' + list.length + ' أمر و ' + irOk + '/' + irs.length + ' إشارة', bad);
    resetForm(); loadIrList().then(loadCommands);
  } catch (e) { toast('فشل الاستيراد', true); }
}

/* ---------- تثبيت كتطبيق (PWA) ---------- */
let deferredInstall = null;
const isStandalone = () => matchMedia('(display-mode: standalone)').matches || navigator.standalone === true;
function pwaUi() {
  const b = $('#pwa-badge'), i = $('#pwa-info'), btn = $('#pwa-btn'), hb = $('#instBtn');
  if (!b) return;
  const ready = !!deferredInstall && !isStandalone();
  btn.hidden = !ready; hb.hidden = !ready;
  if (isStandalone()) { b.textContent = 'مثبّت'; b.className = 'badge ok'; i.textContent = 'التطبيق شغال كتطبيق مستقل.'; return; }
  if (ready) { b.textContent = 'جاهز'; b.className = 'badge ok'; i.textContent = 'اضغط تثبيت وهيتضاف على الشاشة الرئيسية كتطبيق بدل الاختصار.'; return; }
  b.textContent = 'غير مفعّل'; b.className = 'badge bad';
  i.innerHTML = window.isSecureContext
    ? 'المتصفح لسه ما جهزش التثبيت. افتح قايمة المتصفح واختار «تثبيت التطبيق».'
    : 'كروم بيمنع التثبيت الحقيقي على http. على الموبايل افتح <b dir="ltr">chrome://flags/#unsafely-treat-insecure-origin-as-secure</b> وفعّله، وحط <b dir="ltr">http://' + location.host + '</b> في الخانة، وأعد تشغيل كروم، وبعدها زر التثبيت هيظهر هنا.';
}
window.addEventListener('beforeinstallprompt', e => { e.preventDefault(); deferredInstall = e; pwaUi(); });
window.addEventListener('appinstalled', () => { deferredInstall = null; pwaUi(); toast('تم تثبيت التطبيق'); });
async function installApp() {
  if (!deferredInstall) { pwaUi(); return; }
  deferredInstall.prompt();
  try { await deferredInstall.userChoice; } catch (e) {}
  deferredInstall = null; pwaUi();
}
if ('serviceWorker' in navigator) navigator.serviceWorker.register('/sw.js').catch(() => {});

window.onload = () => { applyTheme(document.documentElement.dataset.theme); initCards(); fillPins(); loadIrList().then(() => { resetForm(); loadCommands(); }); loadInfo(); let ho = false; try { ho = localStorage.getItem('heardOpen') === '1'; } catch (e) {} toggleHeard(ho); pwaUi(); let tb = new URLSearchParams(location.search).get('tab'); if (tb === 'ir') { window.__sub = 'ir'; tb = 'maint'; } if (tb && document.querySelector('.tab[data-tab="' + tb + '"]')) showTab(tb); };
</script>
</body>
</html>
)rawliteral";

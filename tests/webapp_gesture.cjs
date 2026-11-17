// Usage: NODE_PATH=$(npm root -g) node webapp_gesture.cjs /path/to/webapp  (checks hold-to-edit trig/accent/fill gesture)
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch(); const pg = await b.newPage({viewport:{width:1100,height:800}});
  const errs=[]; pg.on('pageerror',e=>errs.push(String(e)));
  await pg.goto('file://'+process.argv[2]+'/index.html');
  await pg.click('#go'); await pg.waitForTimeout(800);
  const st = () => pg.evaluate(()=>{const l=ensurePattern(pat).lanes[lane];return {T:l.Triggers.slice(0,8),A:l.Accents.slice(0,8),F:l.Fills.slice(0,8)}});
  const box = async (lbl)=> (await pg.locator(`[aria-label="${lbl}"]`).first().boundingBox());
  const pad=async(n)=>{const b=await box(n);return [b.x+b.width/2,b.y+b.height/2]};
  // clear step 1..4 of lane 0 for clean start
  await pg.evaluate(()=>{const l=ensurePattern(pat).lanes[0];for(const f of['Triggers','Accents','Fills'])l[f]='-'.repeat(l[f].length);});
  const out={};
  // 1 tap empty step 1 -> on
  let [x,y]=await pad('Step 1'); await pg.mouse.move(x,y); await pg.mouse.down(); await pg.mouse.up(); out.tapOn=await st();
  // 2 hold step1 + accent
  await pg.mouse.move(x,y); await pg.mouse.down(); await pg.focus('[aria-label="Accent modifier"]'); await pg.keyboard.press('Enter'); out.holdAcc=await st();
  await pg.focus('[aria-label="Fill retrigger modifier"]'); await pg.keyboard.press('Enter'); out.holdBoth=await st();
  await pg.mouse.up(); out.afterRelease=await st();
  // 3 hold step1 + accent again (subtract) then fill subtract
  await pg.mouse.move(x,y); await pg.mouse.down(); await pg.focus('[aria-label="Accent modifier"]'); await pg.keyboard.press('Enter'); out.subAcc=await st();
  await pg.mouse.up();
  // 4 tap existing without modifier -> off
  await pg.mouse.move(x,y); await pg.mouse.down(); await pg.mouse.up(); out.tapOff=await st();
  // 5 empty step 2: hold + fill only
  [x,y]=await pad('Step 2'); await pg.mouse.move(x,y); await pg.mouse.down(); await pg.focus('[aria-label="Fill retrigger modifier"]'); await pg.keyboard.press('Enter'); await pg.mouse.up(); out.fillOnly=await st();
  // colours
  await pg.evaluate(()=>{const l=ensurePattern(pat).lanes[0];const set=(f,i)=>l[f]=l[f].slice(0,i)+'#'+l[f].slice(i+1);set('Triggers',2);set('Triggers',3);set('Accents',3);set('Triggers',4);set('Fills',4);set('Triggers',5);set('Accents',5);set('Fills',5);screen='home';redraw();});
  await pg.screenshot({path:'/tmp/shot.png'});
  const lcd=await pg.locator('#screen').boundingBox(); await pg.screenshot({path:'/tmp/lcd.png',clip:lcd});
  console.log(JSON.stringify(out,null,1)); console.log('errors',errs);
  await b.close();
})();

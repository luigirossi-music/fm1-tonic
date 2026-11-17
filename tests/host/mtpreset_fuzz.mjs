// SPDX-License-Identifier: GPL-3.0-only
// wip243: compares the page converter (PB.mtpresetToTnp) with a BUILT tools/tnp_pack on 300 random .mtpreset files (needs libpo32, see tools/tnp_pack.c).
//   node tests/host/mtpreset_fuzz.mjs /path/to/tnp_pack      (run from the project folder; not part of run_tests.sh)
import fs from "fs"; import { execFileSync } from "child_process";
const html=fs.readFileSync("web/emu/index.html","utf8");
const m=/\/\/ --- backup protocol: begin[^\n]*\n([\s\S]*?)\/\/ --- backup protocol: end/.exec(html);
const PB=new Function(`"use strict";\n${m[1]}\nreturn PB;`)();
let seed=12345; const rnd=()=>{seed=(seed*1664525+1013904223)>>>0;return seed/4294967296;};
const pick=a=>a[Math.floor(rnd()*a.length)];
const logr=(a,b)=>a*Math.pow(b/a,rnd());
function num(){ // various magnitudes and formats
  const r=rnd();
  if(r<0.1) return "0"; if(r<0.15) return (rnd()*1e-3).toExponential(3);
  return String(+(rnd()*1000).toFixed(pick([0,2,8,13])));
}
function drum(i){
  const mm=pick(["Decay","Sine","Noise"]), ne=pick(["Exp","Lin","Mod"]);
  const L=[];
  L.push(`DistAmt = ${rnd()*100}`,`EQFreq = ${logr(20,20000)}`,`EQGain = ${rnd()*80-40}`,
   `Level = ${rnd()<0.1?"-inf":rnd()*30-30}`,`Mix = ${rnd()*100}`,`ModAmt = ${rnd()*192-96}`,`ModMode = ${mm}`,
   `ModRate = ${mm==="Decay"?(rnd()<0.1?"inf":logr(0.1,3000)):mm==="Sine"?rnd()*2000:rnd()<0.1?0:logr(1,20000)}`,
   `ModVel = ${rnd()*200}`,`NEnvAtk = ${rnd()<0.2?0:logr(0.01,300)}`,`NEnvDcy = ${logr(1,3000)}`,`NEnvMod = ${ne}`,
   `NFilFrq = ${logr(20,20000)}`,`NFilMod = ${pick(["LP","BP","HP"])}`,`NFilQ = ${logr(0.1,10000)}`,`NStereo = On`,
   `NVel = ${rnd()*200}`,`Name = "x ${i}"`,`OscAtk = ${rnd()<0.2?0:logr(0.01,300)}`,`OscDcy = ${logr(1,3000)}`,
   `OscFreq = ${logr(20,20000)}`,`OscVel = ${rnd()*200}`,`OscWave = ${pick(["Sine","Triangle","Saw"])}`,`Output = A`,`Pan = ${rnd()*200-100}`);
  return L;
}
function bitsStr(len){let s="";for(let i=0;i<len;i++)s+=rnd()<0.3?"#":".";return s;}
function preset(){
  const np=1+Math.floor(rnd()*12), T=[];
  T.push("MicroTonicPresetV3: {",`\tFillRate = ${rnd()*10}`,`\tMastVol = ${rnd()*12-6}`,`\tName = "Fuzz ${Math.floor(rnd()*1e6)} (${Math.floor(rnd()*200)}) long name here"`,"\tPattern = a",`\tStepRate = ${pick(["1/16","1/32","1/8T","1/16T","1/4"])}`,`\tSwing = ${rnd()*100}`,`\tTempo = ${60+rnd()*140}`,"\tMutes: { Off, Off, Off, Off, Off, Off, Off, Off }","\tDrumPatches: {");
  for(let v=1;v<=8;v++){T.push(`\t\t${v}: {`);for(const l of drum(v))T.push("\t\t\t"+l);T.push("\t\t}");}
  T.push("\t}","\tPatterns: {");
  for(let p=0;p<np;p++){const len=pick([16,16,32,64,12,48]);T.push(`\t\t${String.fromCharCode(97+p)}: {`,`\t\t\tChained = ${rnd()<0.3}`,`\t\t\tLength = ${len}`);
    for(let v=1;v<=8;v++){T.push(`\t\t\t${v}: {`,`\t\t\t\tAccents = ${bitsStr(len)}`,`\t\t\t\tFills = ${bitsStr(len)}`,`\t\t\t\tTriggers = ${bitsStr(len)}`,"\t\t\t}");}
    T.push("\t\t}");}
  T.push("\t}","}");
  return T.join(rnd()<0.5?"\r\n":"\n")+"\r\n";
}
let bad=0,N=300;
for(let n=0;n<N;n++){
  const t=preset(); fs.writeFileSync("/tmp/mtfuzz.mtpreset",t);
  let c; try{ execFileSync((process.argv[2] || "build/tnp_pack"),["/tmp/mtfuzz.mtpreset","/tmp/mtfuzz.tnp"],{stdio:"pipe"}); c=fs.readFileSync("/tmp/mtfuzz.tnp"); }catch(e){ c=null; }
  let j=null; try{ j=PB.mtpresetToTnp(t,"").blob; }catch(e){ j=null; }
  const same = (c===null&&j===null) || (c&&j&&Buffer.compare(c,Buffer.from(j))===0);
  if(!same){ bad++; if(bad<4){ console.log("MISMATCH",n,c?c.length:null,j?j.length:null); if(c&&j){for(let i=0;i<c.length;i++)if(c[i]!==j[i]){console.log(" first diff byte",i,c[i],j[i]);break;}} } }
}
console.log(`fuzz ${N} presets: ${bad} mismatches`);

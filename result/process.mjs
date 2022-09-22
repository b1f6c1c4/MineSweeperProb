#!/usr/bin/node

import * as readline from 'node:readline';

const rl = readline.createInterface({
  input: process.stdin,
});

const dic = {};

rl.on('line', (input) => {
  const obj = JSON.parse(input);
  if (!(obj.string in dic))
    dic[obj.string] = [0,0,0];
  dic[obj.string][0] += obj.result.pass;
  dic[obj.string][1] += obj.result.pass + obj.result.fail;
  dic[obj.string][2] += obj.exec.cpu * obj.exec.duration;
});

rl.on('close', () => {
  for (const k in dic) {
    if (dic[k][1] % 1e8)
      console.error(`Warning: ${k} is ${dic[k][1]} not N*1e8`);
    dic[k] = {
      r: Math.round(dic[k][0] / dic[k][1] * 1e8) / (1e8 / 100),
      t: Math.round(dic[k][2] / dic[k][1] * 1e8 / 8 / 60),
    };
  }

  const g = (hsh) => dic[hsh] || { r: 'TODO', t: '???' };

  const f = (c) => `
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[1,1]-PSEQ-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-${c}-SFAR 100000000</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-${c}-SFAR`).t} min to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-${c}-SFAR 100000000</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).t} min to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[3,3]-PSEQ-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-${c}-SNR 100000000</pre>
              Approx. takes ${g(`FL@[3,3]-PSEQ-${c}-SNR`).t} min to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[3,3]-PSEQ-D256-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-${c}-SNR 100000000</pre>
              Approx. takes ${g(`FL@[3,3]-PSEQ-D256-${c}-SNR`).t} min to run on an 8-core machine.</details></td>`;

  console.log(`
<table>
    <thead>
        <tr>
            <th>Board Size</th>
            <th>Rule</th>
            <th>Strategy</th>
            <th>Rate</th>
            <th>Cmd</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=4>8x8<br>10 mines</td>${f('8-8-T10')}
        </tr>
        <tr>
            <td rowspan=4>9x9<br>10 mines</td>${f('9-9-T10')}
        </tr>
        <tr>
            <td rowspan=4>16x16<br>40 mines</td>${f('16-16-T40')}
        </tr>
        <tr>
            <td rowspan=4>30x16<br>99 mines</td>${f('30-16-T99')}
        </tr>
    </tbody>
</table>
`);
});


# MineSweeperProb
A deterministic Minesweeper solver

## Build

```bash
sudo pacman -S cmake ninja boost
cmake -S MineSweeperSolver -B cmake-bulid-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-mnative' -G Ninja
cmake --build cmake-build-release
```

## Winning Rate

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
            <td rowspan=4>8x8<br>10 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>81.6351±0.0034%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes 20 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>81.7711±0.0076%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-8-8-T10-SFAR 100000000</pre>
              Approx. takes 250 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>90.1456±0.0026%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>90.3443±0.0099%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-8-8-T10-SNR 100000000</pre>
              Approx. takes 333 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>9x9<br>10 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>91.6775±0.0024%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes 15 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>91.6949±0.0054%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-9-9-T10-SFAR 100000000</pre>
              Approx. takes 24 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>97.1453±0.0015%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>97.1669±0.0033%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-9-9-T10-SNR 100000000</pre>
              Approx. takes 19 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>16x16<br>40 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>78.1228±0.0057%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes 54 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>78.2295±0.0081%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-16-16-T40-SFAR 100000000</pre>
              Approx. takes 116 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>88.9267±0.0031%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>89.0776±0.0095%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-16-16-T40-SNR 100000000</pre>
              Approx. takes 168 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>30x16<br>99 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>39.6123±0.0096%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>40.044±0.014%</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-30-16-T99-SFAR 100000000</pre>
              Approx. takes 547 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>51.8195±0.0098%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>52.496±0.046%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-30-16-T99-SNR 100000000</pre>
              Approx. takes 802 hours to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>



## Winning Rate by First Move


### 8x8, 10 mines
SFAR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>81.6351<br>±0.0034%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes 20 minutes to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

SNR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>88.2804<br>±0.0028%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 16 minutes to run on an 8-core machine.</details></td>
            <td>89.2805<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>89.9019<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8633<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8685<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8998<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.2890<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>88.2833<br>±0.0028%
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 16 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>89.2807<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>89.0536<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8280<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.7546<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.7539<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8306<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.0614<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.2826<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>89.9004<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8287<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.1456<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0137<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>90.0168<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>90.1488<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8283<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.9014<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>89.8625<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.7500<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0168<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8105<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8129<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0168<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>89.7537<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8620<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>89.8673<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.7544<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0189<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>89.8144<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8128<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0225<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>89.7540<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8675<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>89.9024<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8338<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.1495<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>90.0183<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>90.0186<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>90.1479<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8313<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.9022<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>89.2897<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>89.0611<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8260<br>+0.0026-0.0027%
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.7526<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.7548<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.8333<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>89.0660<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.2917<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>88.2820<br>±0.0028%
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 16 minutes to run on an 8-core machine.</details></td>
            <td>89.2876<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>89.8990<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8633<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.8666<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.9019<br>±0.0026%
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 11 minutes to run on an 8-core machine.</details></td>
            <td>89.2942<br>±0.0027%
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 13 minutes to run on an 8-core machine.</details></td>
            <td>88.2877<br>±0.0028%
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 16 minutes to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

### 9x9, 10 mines
SFAR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
            <td>9</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>91.6775<br>±0.0024%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes 15 minutes to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,2]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,3]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,4]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,5]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,6]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,7]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,8]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>9</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,9]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

SNR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
            <td>9</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>96.2181<br>±0.0017%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 12 minutes to run on an 8-core machine.</details></td>
            <td>96.7571<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>97.0227<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9532<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7147<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9520<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0240<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7571<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>96.2175<br>±0.0017%
            <details><pre>./MineSweeperSolver FL@[9,1]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 12 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>96.7581<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>96.7878<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0653<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9619<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5497<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9624<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0671<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.7888<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7586<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[9,2]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>97.0212<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0644<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.1453<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0102<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5776<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0101<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.1472<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0640<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0223<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[9,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>96.9511<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9618<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0104<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.8457<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5243<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.8472<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0124<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9609<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9513<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[9,4]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>96.7144<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.5502<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5788<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5254<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.2491<br>±0.0017%
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5238<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5803<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5509<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.7165<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[9,5]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>96.9519<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9620<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0105<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.8482<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5251<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.8485<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0117<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9620<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9513<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[9,6]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>97.0244<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0671<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.1489<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0132<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5810<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0132<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.1507<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0677<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0260<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[9,7]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>96.7594<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>96.7884<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0629<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9636<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.5517<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.9631<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>97.0684<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
            <td>96.7920<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7614<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[9,8]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>9</th>
            <td>96.2182<br>±0.0017%
            <details><pre>./MineSweeperSolver FL@[1,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 12 minutes to run on an 8-core machine.</details></td>
            <td>96.7584<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[2,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>97.0229<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[3,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9515<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[4,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7145<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[5,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.9532<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[6,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>97.0258<br>±0.0015%
            <details><pre>./MineSweeperSolver FL@[7,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 9 minutes to run on an 8-core machine.</details></td>
            <td>96.7614<br>±0.0016%
            <details><pre>./MineSweeperSolver FL@[8,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
            <td>96.2222<br>±0.0017%
            <details><pre>./MineSweeperSolver FL@[9,9]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 12 minutes to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

### 16x16, 40 mines
SFAR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>78.1228<br>±0.0057%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes 54 minutes to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

SNR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>85.4942<br>±0.0049%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 50 minutes to run on an 8-core machine.</details></td>
            <td>87.0982<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 45 minutes to run on an 8-core machine.</details></td>
            <td>87.7811<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 43 minutes to run on an 8-core machine.</details></td>
            <td>87.8495<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.4703<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.5119<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.5402<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.4926<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>87.0977<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 45 minutes to run on an 8-core machine.</details></td>
            <td>87.8506<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 43 minutes to run on an 8-core machine.</details></td>
            <td>88.5908<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 41 minutes to run on an 8-core machine.</details></td>
            <td>88.5989<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
            <td>87.9047<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
            <td>87.9162<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.9331<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.8598<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>87.7933<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 43 minutes to run on an 8-core machine.</details></td>
            <td>88.5926<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 41 minutes to run on an 8-core machine.</details></td>
            <td>88.9267<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
            <td>88.8583<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.1090<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0952<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.1091<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>88.0294<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>87.8463<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>88.5942<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
            <td>88.8635<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.7111<br>±0.0031%
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0770<br>±0.0032%
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0451<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>88.0402<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>87.9902<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>87.4697<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.9034<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
            <td>88.1142<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0816<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5345<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5308<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5319<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4839<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>87.5105<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 42 minutes to run on an 8-core machine.</details></td>
            <td>87.9166<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.1082<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0521<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>87.5350<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5167<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5226<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4753<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>87.5398<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 43 minutes to run on an 8-core machine.</details></td>
            <td>87.9338<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.1172<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0442<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>87.5304<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5179<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.5215<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4732<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>87.4916<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 43 minutes to run on an 8-core machine.</details></td>
            <td>87.8641<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>88.0401<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.9901<br>±0.0045%
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 38 minutes to run on an 8-core machine.</details></td>
            <td>87.4774<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4774<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4768<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
            <td>87.4350<br>±0.0046%
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 39 minutes to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

### 30x16, 99 mines
SFAR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
            <td>9</td>
            <td>10</td>
            <td>11</td>
            <td>12</td>
            <td>13</td>
            <td>14</td>
            <td>15</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>39.6123<br>±0.0096%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,2]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,3]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,4]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,5]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,6]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,7]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,8]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>

SNR

<table>
    <thead>
        <tr>
            <th></th>
            <td>1</td>
            <td>2</td>
            <td>3</td>
            <td>4</td>
            <td>5</td>
            <td>6</td>
            <td>7</td>
            <td>8</td>
            <td>9</td>
            <td>10</td>
            <td>11</td>
            <td>12</td>
            <td>13</td>
            <td>14</td>
            <td>15</td>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>1</th>
            <td>46.7470<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>48.9188<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>50.0949<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>50.3687<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>49.9929<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,1]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>2</th>
            <td>50.1038<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>48.9491<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>50.0923<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>51.4913<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>52.4287<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,2]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>3</th>
            <td>52.7909<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>52.6958<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>51.8195<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
            <td>50.3836<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>50.0144<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>4</th>
            <td>51.4280<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 9 hours to run on an 8-core machine.</details></td>
            <td>51.3606<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>52.1141<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>51.9813<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>51.2316<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 8 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[6,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[7,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,4]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>5</th>
            <td>51.3558<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 11 hours to run on an 8-core machine.</details></td>
            <td>51.4442<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 10 hours to run on an 8-core machine.</details></td>
            <td>51.4124<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 10 hours to run on an 8-core machine.</details></td>
            <td>51.3256<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 10 hours to run on an 8-core machine.</details></td>
            <td>52.0896<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 9 hours to run on an 8-core machine.</details></td>
            <td>51.9536<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[6,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 9 hours to run on an 8-core machine.</details></td>
            <td>51.2470<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[7,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 9 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,5]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>6</th>
            <td>51.4572<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.3769<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.3664<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.4563<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.4203<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 11 hours to run on an 8-core machine.</details></td>
            <td>51.3497<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[6,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 11 hours to run on an 8-core machine.</details></td>
            <td>52.0996<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[7,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 10 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,6]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>7</th>
            <td>51.3401<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.4086<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.4541<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>51.3696<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>51.3769<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 16 hours to run on an 8-core machine.</details></td>
            <td>51.4497<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[6,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>51.4142<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[7,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,7]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <th>8</th>
            <td>51.9735<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[1,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 11 hours to run on an 8-core machine.</details></td>
            <td>52.1024<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[2,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 11 hours to run on an 8-core machine.</details></td>
            <td>51.3548<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[3,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 12 hours to run on an 8-core machine.</details></td>
            <td>51.4243<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[4,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>51.4519<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[5,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>51.3849<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[6,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 14 hours to run on an 8-core machine.</details></td>
            <td>51.3838<br>±0.0098%
            <details><pre>./MineSweeperSolver FL@[7,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 13 hours to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[8,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[9,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[10,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[11,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[12,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[13,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[14,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
            <td>TODO
            <details><pre>./MineSweeperSolver FL@[15,8]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>


import React from 'react';

export default function Block(props) {
    const {
        isGameOver,
        isWon,
        isOpen,
        degree,
        isFlagged,
        isEFlagged,
        hasMine,
        probability,
        isBest,
        isPreferred,
        isSafe,
        isDangerous,
        isDrain,
        row,
        col,
        onProbe,
        onFlag,
    } = props;

    let tdClass = 'blk';
    let tdStyle = {};
    let spanClass = '';
    let text = '';

    if (isOpen) {
        tdClass += ' blk-open';
        if (hasMine) {
            tdClass += ' blk-dead';
            text = 'M';
        } else if (isEFlagged) {
            spanClass = 'flag';
            text = 'e';
        } else if (degree) {
            spanClass = `num-${degree}`;
            text = degree;
        }
    } else if (!isGameOver && !isWon) {
        if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        }
        if (isSafe)
            tdClass += ' blk-safe';
        else if (isDangerous)
            tdClass += ' blk-dangerous';
        else if (probability !== null) {
            if (!isDrain) {
                const r = Math.round(0xf8 * probability + 0x09 * (1 - probability));
                const g = Math.round(0x29 * probability + 0xb7 * (1 - probability));
                const b = Math.round(0x29 * probability + 0x22 * (1 - probability));
                tdStyle.backgroundImage = `linear-gradient(135deg, rgb(${r}, ${g}, ${b}), #d2d2d2 40%)`;
            } else {
                const r = Math.round(0xf8 * probability + 0x7b * (1 - probability));
                const g = Math.round(0x29 * probability + 0x68 * (1 - probability));
                const b = Math.round(0x29 * probability + 0xee * (1 - probability));
                tdStyle.backgroundImage = `linear-gradient(45deg, rgb(${r}, ${g}, ${b}), transparent 100%)`;
            }
        }
        if (isBest)
            tdClass += ' blk-best';
        if (isPreferred)
            tdClass += ' blk-preferred';
        if (isDrain)
            tdClass += ' blk-drain';
    } else {
        if (isFlagged && !hasMine) {
            spanClass = 'flag bad-flag';
            text = 'F';
        } else if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        } else if (!isWon && hasMine) {
            text = 'M';
        }
        if (isWon)
            tdClass += ' blk-won';
    }

    function onClick(e) {
        if (e.type === 'click')
            onProbe(row, col);
        else if (e.type === 'contextmenu') {
            onFlag(row, col);
            e.preventDefault();
        }
    }

    return (<td className={tdClass}
                style={tdStyle}
                onClick={onClick}
                onContextMenu={onClick}>
        <span className={spanClass}>{text}</span>
    </td>);
}

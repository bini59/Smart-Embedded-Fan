document.addEventListener('DOMContentLoaded', function () {
  const powerLevelBtns = document.querySelectorAll('.power-level-btn');
  const rotationSwitch = document.getElementById('rotation-switch');
  const timerInput = document.getElementById('timer-input');
  const timerDisplay = document.getElementById('timer-display');
  const timerBtn = document.getElementById('timer-btn');
  const autoBtn = document.querySelectorAll('.auto-btn');

  powerLevelBtns.forEach((btn, index) => {
    btn.addEventListener('click', () => {
      powerLevelBtns.forEach((btn) => btn.classList.remove('active'));
      btn.classList.add('active');
    });
  });

  autoBtn.forEach((btn, index) => {
    btn.addEventListener('click', () => {
      if (btn.classList.contains('active')) btn.classList.remove('active');
      else btn.classList.add('active');
    });
  });

  rotationSwitch.addEventListener('change', () => {
    
  });

  timerBtn.addEventListener('click', () => {
    let value = timerInput.value;
    if (value === '') {
      value = 0;
    }

    timerInput.value = '';
    timerDisplay.innerText = `남은 시간 : ${value}분`;
  });
});

const corner = document.getElementById('corner');
function resizeWindow(e) {
  const size = {
    width: Math.max(50,Math.floor(e.clientX+5)),
    height: Math.max(50,Math.floor(e.clientY+5))
  };
  parent.postMessage( { pluginMessage: { type: 'resize', size: size }}, '*');
}
corner.onpointerdown = (e)=>{
  corner.onpointermove = resizeWindow;
  corner.setPointerCapture(e.pointerId);
};
corner.onpointerup = (e)=>{
  corner.onpointermove = null;
  corner.releasePointerCapture(e.pointerId);
};


/**
 * api part
 */

document.addEventListener('DOMContentLoaded', function () {

  const powerLevelBtns = document.querySelectorAll('.power-level-btn');
  const rotationSwitch = document.getElementById('rotation-switch');
  const timerInput = document.getElementById('timer-input');
  const timerBtn = document.getElementById('timer-btn');
  const timerDisplay = document.getElementById('timer-display');

  fetch('/api/v1/power')
    .then((res) => res.json())
    .then((power) => {
      powerLevelBtns[power - 1].classList.add('active');
    });

  powerLevelBtns.forEach((btn, index) => {
    btn.addEventListener('click', () => {
      const power = index + 1;
      fetch('/api/v1/power', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ power }),
      });
    });
  });

  fetch('/api/v1/rotation')
    .then((res) => res.json())
    .then((rotation) => {
      rotationSwitch.checked = rotation;
    });

  rotationSwitch.addEventListener('change', () => {
    const rotation = rotationSwitch.checked;
    fetch('/api/v1/rotation', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ rotation }),
    });
  });

  fetch('/api/v1/timer')
    .then((res) => res.json())
    .then((timer) => {
      timerDisplay.innerText = `남은 시간 : ${timer}분`;
    });

  timerBtn.addEventListener('click', () => {
    let value = timerInput.value;
    if (value === '') {
      value = 0;
    }
    fetch('/api/v1/timer', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ timer: value }),
    });
  });
});
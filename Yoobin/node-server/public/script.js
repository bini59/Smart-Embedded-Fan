const initBtns = () => {

  // power 버튼 초기화
  const initPowerBtns = () => {

    const powerAutoSwitch = document.getElementById('power-auto-switch');

    const powerBtns = document.querySelectorAll('.power-level-btn');
    powerBtns.forEach((btn) => {
      btn.addEventListener('click', () => {
        // 자동 모드일 시 버튼 클릭 무시
        if (powerAutoSwitch.checked) return;
        powerBtns.forEach((btn) => btn.classList.remove('active'));
        btn.classList.add('active');
      });
    })

    // 현재 파워 상태 가져오기
    fetch('/api/v1/power')
      .then((res) => res.json())
      .then((power) => {
        powerBtns[power - 1].classList.add('active');
      });
      

    // 파워 버튼 클릭 시
    powerBtns.forEach((btn, index) => {
      btn.addEventListener('click', () => {
        // 자동 모드일 시 버튼 클릭 무시
        if (powerAutoSwitch.checked) return;
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

    console.log('init power btn is done!');
  }


  const initRotationBtn = () => {
    const rotationSwitch = document.getElementById('rotation-switch');

    // 현재 회전 상태 가져오기
    fetch('/api/v1/rotation')
      .then((res) => res.json())
      .then((rotation) => {
        rotationSwitch.checked = rotation;
      });

    // 회전 버튼 클릭 시
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
  }

  const initTimerBtn = () => {
    const timerBtn = document.querySelectorAll('.timer-level-btn');

    timerBtn.forEach((btn, index) => {
      btn.addEventListener('click', () => {
        timerBtn.forEach((btn) => btn.classList.remove('active'));
        btn.classList.add('active');
      })
    });

    timerBtn.forEach((btn, index) => {
      btn.addEventListener('click', () => {
        const timer = index + 1
        fetch('/api/v1/timer', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({timer}),
        })
      })
    });
  }

  const initAutoBtn = () => {
    const autoBtns = document.querySelectorAll('.auto-check');
    const powerBtns = document.querySelectorAll('.power-level-btn');

    const powerAutoSwitch = document.getElementById('power-auto-switch');
    const roatateAutoSwitch = document.getElementById('rotation-auto-switch');

    fetch('/api/v1/auto/power')
    .then((res) => res.json())
    .then((data) => {
        powerAutoSwitch.checked = data == 1 ? true : false;
        if (data == 1) powerBtns.forEach((subBtn) => subBtn.classList.add('disabled'));
    })

    fetch('/api/v1/auto/rotate')
    .then((res) => res.json())
    .then((data) => {
        roatateAutoSwitch.checked = data == 1 ? true : false;
      })
      
    // auto btn interactive event
    autoBtns.forEach((btn) => {
      btn.addEventListener('change', (e)=>{
        let target = e.target;
        if(btn.id !== 'power-auto-switch') return;
  
        if(target.checked) powerBtns.forEach((subBtn) => subBtn.classList.add('disabled'));
        else powerBtns.forEach((subBtn) => subBtn.classList.remove('disabled'));
      })
    })

    powerAutoSwitch.addEventListener('click', (e) => {
      const auto  = e.target.checked == true ? 1 : 0;
      fetch('/api/v1/auto/power', {
        method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ auto }),
      })
    });

    roatateAutoSwitch.addEventListener('click', (e) => {
      const auto  = e.target.checked == true ? 1 : 0;
      fetch('/api/v1/auto/rotate', {
        method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ auto }),
      })
    })
  }

  return [
    initPowerBtns, initRotationBtn, initTimerBtn, initAutoBtn
  ]
}

const init = () => {
  const initInteractiveMethods = initBtns();
  initInteractiveMethods.forEach((method)=>method());
}

const powerLevelBtnClickMethod = (btns, btn) => {
  btns.forEach((btn) => btn.classList.remove('active'));
  btn.classList.add('active');
}

document.addEventListener('DOMContentLoaded', ()=>{
  init();
})

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
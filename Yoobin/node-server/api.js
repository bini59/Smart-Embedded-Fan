// app has 3 api
/**
 * 1. /api/v1/power
 * get : get power level
 * post : set power level
 * 
 * 2. /api/v1/rotation
 * get : get rotation status
 * post : set rotation status
 * 
 * 3. /api/v1/timer
 * get : get timer status
 * post : set timer status
 * 
 * 
 */

const router = require('express').Router();

const { getData, setData } = require('./module/util');

router.get('/power', (req, res) => {
  const power = getData('power');
  res.send(power);
});

router.post('/power', (req, res) => {
  const power = req.body.power;
  try{
    setData('power', power);
  }
  catch(e){
    res.send({error: e})
  }
  res.send({power})
});

router.get('/rotation', (req, res) => {
  const rotation = getData('rotation');
  res.send(rotation == 1 ? true : false);
});

router.post('/rotation', (req, res) => {
  const rotation = req.body.rotation;
  try{
    setData('rotation', rotation ? 1 : 0);
  }
  catch(e){
    res.send({error: e})
  }
  res.send({rotation})
});

router.get('/timer', (req, res) => {
  const timer = getData('timer');
  res.send(timer);
});

router.post('/timer', (req, res) => {
  const timer = req.body.timer;
  try{

    setData('timer', timer);
  }
  catch(e){
    res.send({error: e})
  }
  res.send({timer})
});


router.get('/auto/power', (req, res) => {
  const powerAuto = getData('powerAuto');
  res.send(powerAuto);
})

router.post('/auto/power', (req, res) => {
  const auto = req.body.auto;
  res.send({auto});
})

router.get('/auto/rotate', (req, res) => {
  const powerAuto = getData('rotateAuto');
  res.send(powerAuto);
})

router.post('/auto/rotate', (req, res) => {
  const auto = req.body.auto;
  res.send({auto});
})



module.exports = router;
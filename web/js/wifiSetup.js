var esp = esp || {};

esp.wifiSetup = (function() {
    var apScanUrl = '/api/apscan';
    var setupUrl = '/api/setup';
    var elements = { };

    function isApRow(el) {
      return esp.share.hasClass('ap-row', el);
    }

    document.addEventListener("DOMContentLoaded", function(event) { 
        elements.content = document.getElementById('content');
        elements.apTable = document.getElementById('wifi-aplist');
        elements.refreshAps = document.getElementById('refresh-aps');
        elements.refreshAps.addEventListener('click', sendRefresh);
        
        elements.setAp = document.getElementById('set-ap');
        elements.setAp.addEventListener('click', sendSetup);
        elements.setAp.addEventListener("submit", function (evt) {
            evt.preventDefault();
            return false;
        });

        elements.ssid = document.getElementById('ap-ssid');
        elements.password = document.getElementById('ap-password');
        elements.mqttHost = document.getElementById('mqtt-host');
        elements.mqttPort = document.getElementById('mqtt-port');
        elements.loading = document.getElementById('loading');

        document.body.addEventListener('click', function(evt) {
          var row = isApRow(evt.target) ? evt.target : esp.share.findAncestor(isApRow, evt.target);

          if(row) {
            rowClick(row);
          }
        });

        elements.loading.style.display = 'none';
        elements.content.style.display = '';
        
        getSetup();
    });

    var auth = {
        open: 0,
        wep: 1,
        wpaPsk: 2,
        wpa2Psk: 3,
        wpaWpa2Psk: 4,
        max: 5
    };
    
    function setMuiField(element, value) {
        element.value = value;
        var evt = document.createEvent("HTMLEvents");
        evt.initEvent("change", false, true);
        element.dispatchEvent(evt);
    }

    function anyTrueAnscestors(element, fn) {
      if(fn(element)) {
        return true;
      }
      
      return element.parentElement ?
        anyTrueAnscestors(element.parentElement, fn) :
        false;
    }

    
    function rowClick(row) {
      var isSelected = esp.share.hasClass('ap-selected', row);
      var tbody = elements.apTable.getElementsByTagName('tbody')[0];
      var rows = [].slice.call(tbody.children);

      rows.forEach(function(r) {
        esp.share.removeClass('ap-selected', r);
      });

      if(!isSelected) {
        esp.share.addClass('ap-selected', row);
        var apObj = getAp(row.getAttribute('ap-id'));
        elements.ssid.value = apObj.ssid;
        
        var evt = document.createEvent("HTMLEvents");
        evt.initEvent("change", false, true);
        elements.ssid.dispatchEvent(evt);
      }
    }

    function scanError() {
      clearApList();
      //TODO error...
    }

    function getSetup() {
        esp.share.post(setupUrl, {type: 'get'})
        //getTest()
          .then(function(data){
            setMuiField(elements.ssid, data.ssid);
            setMuiField(elements.mqttHost, data.mqttHost);
            setMuiField(elements.mqttPort, data.mqttPort);
          });
    }

    function sendSetup() {
      var payload = {
        type: 'set',
        setup: {
          ssid: elements.ssid.value,
          password: elements.password.value,
          mqttHost: elements.mqttHost.value,
          mqttPort: parseInt(elements.mqttPort.value),
        }
      };

      esp.share.post(setupUrl, payload)
      //setTest()
        .then(function _success(data) {
          if(data.error) {
            activateModal('Error: ' + data.error);
          } else {
            activateModal('AP updated!');
          }
        }, function _fail(){})
    }

    function sendRefresh() {
      clearApList();
      var loading = elements.loading.cloneNode(true);
      loading.style.display = '';
      elements.apTable.appendChild(crel('tbody',loading));

      var pollLoop = null;
      pollLoop = function() {
        setTimeout(function() {
          esp.share.post(apScanUrl, {aplist: 'list'})
          //testRefreshDone()
            .then(function(data) {
              if(data.status === 'SCANNING')  {
                pollLoop();
              } else if(data.status === 'SCAN ERROR') {
                scanError();
              } else if(data.status === 'FINISHED') {
                populateApList(data.apList || []);
              }
            });
        }, 2000);
      };

      esp.share.post(apScanUrl, {aplist: 'refresh'})
      //testRefreshStart()
        .then(function(response) {
          if(response.status === 'SCANNING') {
            pollLoop();
          }
        });
    }

    function askForList() {
        return esp.share.post(url, {aplist: 'list'});
    }

    function clearApList(){
        var tbody = elements.apTable.getElementsByTagName('tbody')[0];
        if(tbody) {
          tbody.remove();
        }
    }

    function populateApList(apList) {
        clearApList();
        var rows = null;
        rows = crel('tbody', apList.map(function(ap, idx){
          return crel('tr', {'class': 'ap-row', 'ap-id': idx}, 
            crel('td', {'class': 'col-ssid'}, ap.ssid),
            crel('td', {'class': 'col-strength'}, ap.strength),
            crel('td', {'class': 'col-auth'}, ap.auth === 0 ? "Open" : "Secured"))
          }));
        elements.apTable.appendChild(rows); 
    }

    function getAp(idx) {
      //return data.apList[idx];
      return {ssid:"TODO"};
    }


    function showReset() {}
      
    function activateModalMessage(message) {
      var modalEl = crel('div', {'class': 'modalMain'},
        crel('div', {'class': 'modalMsg'}, message));
      // show modal
      mui.overlay('on', modalEl); 
    }

    return {
        
    };

    function testRefreshStart() {
      return new Promise(function(resolve,reject){
        resolve({"status":"SCANNING","apList":[]});
      });
    }

    function testRefreshDone() {
      return new Promise(function(resolve,reject){
        resolve({"status":"FINISHED","apList":[{"strength":-92,"ssid":"Gaedeclarke95","auth":4},{"strength":-86,"ssid":"GaedekeClarke2","auth":4},{"strength":-92,"ssid":"HOME-7D3C-2.4","auth":4},{"strength":-46,"ssid":"Asus_Joes","auth":3}]});  
      });
    }

    function setTest() {
      return new Promise(function(resolve, reject) {
        resolve({"type":"set", "setup":{ "ssid":"Asus_Joes", "password":"fuckingshit", "mqttHost":"pihub.home.lan", "mqttPort":1880}});
      });
    }
    function getTest() {
      return new Promise(function(resolve, reject) {
        resolve({"ssid":"Asus_Joes", "mqttHost":"pihub.home.lan", "mqttPort":1880});
      });
    }
}());
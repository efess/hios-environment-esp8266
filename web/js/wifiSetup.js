var esp = esp || {};
esp.wifiSetup = (function() {
    var url = '/api/apscan';
    function isApRow(el) {
      return esp.share.hasClass('ap-row', el);
    }

    document.addEventListener("DOMContentLoaded", function(event) { 
        elements.refreshAps = document.getElementById('refresh-aps');
        elements.refreshAps.addEventListener('click', sendRefresh);

        elements.apTable = document.getElementById('wifi-aplist');
        elements.ssid = document.getElementById('ap-ssid');
        elements.password = document.getElementById('ap-password')
        elements.loading = document.getElementById('loading');

        document.body.addEventListener('click', function(evt) {
          var row = isApRow(evt.target) ? evt.target : esp.share.findAncestor(isApRow, evt.target);

          if(row) {
            rowClick(row);
          }
        });
    });

    var auth = {
        open: 0,
        wep: 1,
        wpaPsk: 2,
        wpa2Psk: 3,
        wpaWpa2Psk: 4,
        max: 5
    }

    function anyTrueAnscestors(element, fn) {
      if(fn(element)) {
        return true;
      }
      
      return element.parentElement ?
        anyTrueAnscestors(element.parentElement, fn) :
        false;
    }

    var elements = { };

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

    function sendRefresh() {
      clearApList();
      var loading = elements.loading.cloneNode(true);
      loading.style.display = '';
      elements.apTable.appendChild(crel('tbody',loading));

      var pollLoop = null;
      pollLoop = function() {
        setTimeout(function() {
          esp.share.post(url, {aplist: 'list'})
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

      esp.share.post(url, {aplist: 'refresh'})
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
            crel('td', ap.ssid),
            crel('td', ap.strength))
          }));
        elements.apTable.appendChild(rows); 
    }

    function getAp(idx) {
      //return data.apList[idx];
      return {ssid:"TODO"};
    }
    
    return {
        
    };
}());
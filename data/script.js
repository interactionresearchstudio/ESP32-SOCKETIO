function init() {
    $.getJSON('/credentials', function (json) {
        configure(json);
    });
}

function configure(json) {
    console.log(json);

    $('#save-button').click(onSaveButtonClicked);
    $('#save-button').click(onSaveButtonClicked);

    $('#local_ssid').keypress(onKeyPressed);
    $('#local_pass').keypress(onKeyPressed);

    $('#local_ssid').text(json.local_ssid);
    //TODO: use the password length to display
     $('#remote-scad-code').text(formatScadCode(json.remote_mac));
     $('#local-scad-code').text(formatScadCode(json.local_mac));

    if(json.local_paired_status == "remoteSetup") {
        //Show local wifi form, local and remote IDs
        var remoteWifi = document.getElementById("remoteWifiForm");
        remoteWifi.style.display = "none";
        var remoteMac = document.getElementById("remoteMacForm");
        remoteMac.style.display = "block";
    } else if(json.local_paired_status == "localSetup"){
        //Show local and remote wifi form
        var remoteWifi = document.getElementById("remoteWifiForm");
        remoteWifi.style.display = "block";
        //var remoteMacInput = document.getElementById("remoteMacForm-input");
        //remoteMacInput.style.display = "none";
        var remoteMac = document.getElementById("remoteMacForm");
        remoteMac.style.display = "block";
    } else if(json.local_paired_status == "pairedSetup"){
        //just show local wifi details
        var remoteWifi = document.getElementById("remoteWifiForm");
        remoteWifi.style.display = "none";
        var remoteMac = document.getElementById("remoteMacForm");
        remoteMac.style.display = "block";
        //var remoteMacInput = document.getElementById("remoteMacForm-input");
        //remoteMacInput.style.display = "none";
    } 

    $('#local_ssid').attr('disabled', true);
    populateNetworksList();
}

function formatScadCode(code) {
    return(code.slice(0, 4) + ' ' + code.slice(4));
}

function onKeyPressed(event) {
    if (event.keyCode == 13) {
        event.preventDefault();
        onSaveButtonClicked();
    }
}

function populateNetworksList() {
    let networks = $('#networks-list-select');
    networks.empty();

    $.getJSON('/scan', function (json) {
        $.each(json, function (key, entry) {
            networks.append($('<option></option>').attr('value', entry.SSID));
        });
        $('#local_ssid').attr('disabled', false);
    });
}

function onSaveButtonClicked() {
    var data = {
        local_ssid: $('#local_ssid').val(),
        local_pass: $('#local_pass').val(),
        remote_mac: $('#remote-scad-code-input').val().replace(/\s/g,''),
        remote_ssid: $('#remote_ssid').val(),
        remote_pass: $('#remote_pass').val()
    };
    
    console.log(data);

    $.ajax({
        type: "POST",
        url: "/credentials",
        data: JSON.stringify(data),
        dataType: 'json',
        contentType: 'application/json; charset=utf-8'
    }).done(function(response) {
        // handle a successful response
    }).fail(function(xhr, status, message) {
        // handle a failure response
    });
}
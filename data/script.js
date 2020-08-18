function init() {
    $.getJSON('/credentials', function (json) {
        configure(json);
    });
}

function configure(json) {
    console.log(json);

    $('#save-button').click(onSaveButtonClicked);
    $('#save-button').click(onSaveButtonClicked);

    $('#network').keypress(onKeyPressed);
    $('#password').keypress(onKeyPressed);

    $('#local-scad-code').text(json.local_mac);
    $('#remote-scad-code').text(json.remote_mac);

    $('#network').text(json.local_ssid);
    //TODO: use the password length to display

    if(json.local_paired_status == "pairing") {
        $('#remoteWifiForm').attr('display', "block");
    }
    else{
        $('#remoteWifiForm').attr('display', "none");
    }

    $('#networks-dropdown').change(onNetworksDropdownChanged);
    $('#networks-dropdown').attr('disabled', true);

    populateNetworksList();
}

function onKeyPressed(event) {
    if (event.keyCode == 13) {
        event.preventDefault();
        onSaveButtonClicked();
    }
}

function populateNetworksList() {
    let dropdown = $('#networks-dropdown');
    let list = $('#networks-list');

    dropdown.empty();

    dropdown.append('<option selected="true" disabled>Choose Network</option>');
    dropdown.prop('selectedIndex', 0);

    $.getJSON('/scan', function (json) {
        $.each(json, function (key, entry) {
            dropdown.append($('<option></option>').attr('value', entry.SSID).text(entry.SSID));
            dropdown.append($('<p></p>').text(entry.SSID));
        });
        $('#networks-dropdown').attr('disabled', false);
    });
}

function onNetworksDropdownChanged() {
    $('#local_ssid').val($("#networks-dropdown :selected").text());
}

function onSaveButtonClicked() {
    var data = {
        local_ssid: $('#local_ssid').val(),
        local_pass: $('#local_pass').val(),
        remote_mac: $('#remote-scad-code').val(),
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
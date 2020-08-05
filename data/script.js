function init() {
    $('#save-button').click(onSaveButtonClicked);
    $('#networks-dropdown').change(onNetworksDropdownChanged);
    $('#networks-dropdown').attr('disabled', true);
    populateNetworksList();
}

function populateNetworksList() {
    let dropdown = $('#networks-dropdown');
    let list = $('#networks-list');

    dropdown.empty();

    dropdown.append('<option selected="true" disabled>Choose Network</option>');
    dropdown.prop('selectedIndex', 0);

    $.getJSON('/scan', function (data) {
    $.each(data, function (key, entry) {
        dropdown.append($('<option></option>').attr('value', entry.SSID).text(entry.SSID));
        dropdown.append($('<p></p>').text(entry.SSID));
    });
    $('#networks-dropdown').attr('disabled', false);
    });
}

function onNetworksDropdownChanged() {
    $('#network').val($("#networks-dropdown :selected").text());
}

function onSaveButtonClicked() {
    var data = {
        network: {
            ssid: $('#network').val(),
            password: $('#password').val()
        }
    };

    $.ajax({
        type: "POST",
        url: "/settings",
        data: JSON.stringify(data),
        dataType: 'json',
        contentType: 'application/json; charset=utf-8'
    }).done(function(response) {
        // handle a successful response
    }).fail(function(xhr, status, message) {
        // handle a failure response
    });
}

export async function get({request}) {
    return {
        status: 200,
        body: {
            "enableble": true,
            "enabledac": false,
            "enablemqtt": true,
            "enablesoftap": true,
            "enablewifi": true,
            "hostname": "freshwater",
            "mqtthost": "192.168.255.1",
            "mqttpass": "abcd1234",
            "mqttport": 1883,
            "mqtttopic": "freshwater",
            "mqttuser": "freshwater",
            "otaPassword": "0123456789"
        }
    }
}

export async function post({request}) {
    return {
        status: 200,
        body: {
            "message": "success"
        }
    }
}
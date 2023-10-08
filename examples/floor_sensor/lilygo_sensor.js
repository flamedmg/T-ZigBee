const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const extend = require('zigbee-herdsman-converters/lib/extend');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
    zigbeeModel: ['DmG.FloorSensor'],
    model: 'DmG.FloorSensor',
    vendor: 'DmG',
    description: 'heated floor temperature sensor for 2 zones',
    fromZigbee: [fz.temperature, fz.temperature],
    toZigbee: [],
    exposes: [e.temperature(), e.temperature()],
    // The configure method below is needed to make the device reports on/off state changes
    // when the device is controlled manually through the button on it.
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['msTemperatureMeasurement', 'msTemperatureMeasurement']);
        await reporting.temperature(endpoint);
        await reporting.temperature(endpoint);
    },
};

module.exports = definition;

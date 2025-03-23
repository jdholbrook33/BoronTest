# Pool Flow Monitor - Particle Boron Implementation

## Overview
This project uses a Particle Boron device to monitor pool water flow, calculate gallons used, and send data to AWS DynamoDB for analysis and visualization.

## Hardware
- Particle Boron (LTE cellular IoT device)
- Flow meter sensor connected to D2 pin
- Status LED on D7 pin

## Key Specifications
- Flow meter pulse rate: 476 pulses per gallon
- Fill event timeout: 20 seconds (configurable)
- Data transmission: Only upon fill event completion

## Device Code
The Boron monitors pulses from a flow sensor, calculates gallons used, and publishes data to the Particle cloud when a fill event completes.

Key features:
- Interrupt-driven pulse counting
- Automatic fill event detection
- LED indicator for active flow
- Cellular connectivity via Particle

## Data Pipeline
1. Boron device detects fill events
2. Device publishes JSON payload to Particle cloud
3. Particle webhook forwards data to AWS API Gateway
4. API Gateway triggers Lambda function
5. Lambda writes data to DynamoDB

## AWS Resources
- **API Gateway**: Endpoint for Particle webhook
- **Lambda Function**: Processes data and writes to DynamoDB
- **DynamoDB Table**: 
  - Table Name: PoolFlowData
  - Partition Key: deviceId (String)
  - Sort Key: timestamp (Number)

## JSON Payload Format
```json
{
  "device_id": "pool_1",
  "timestamp": 1647534963,
  "gallons_used": 25.75,
  "signal_strength": 40
}
```

## Particle Integration Setup
1. In Particle Console, go to Integrations → New Integration → Webhook
2. Configure webhook:
   - Event Name: "flow_data"
   - URL: [Your API Gateway URL]
   - Request Type: POST
   - Request Format: JSON
   - Include API Key in HTTP Headers (if using API Gateway)

## Lambda Function
The Lambda function processes the Particle webhook payload and stores it in DynamoDB:

```javascript
const AWS = require('aws-sdk');
const dynamoDB = new AWS.DynamoDB.DocumentClient();

exports.handler = async (event) => {
    const body = JSON.parse(event.body);
    const deviceData = JSON.parse(body.data);
    
    const params = {
        TableName: 'PoolFlowData',
        Item: {
            deviceId: deviceData.device_id,
            timestamp: Number(deviceData.timestamp),
            gallonsUsed: Number(deviceData.gallons_used),
            signalStrength: Number(deviceData.signal_strength)
        }
    };
    
    await dynamoDB.put(params).promise();
    
    return { 
        statusCode: 200, 
        body: JSON.stringify({success: true}) 
    };
};
```

## Notes
- The first 100 Particle devices can use the platform for free
- Data usage is minimal (~18KB/month per device)
- Consider B-SoM ($46) for production devices
- Cellular connectivity is managed automatically by Particle


ARN - arn:aws:execute-api:us-east-2:038462771096:kp5m4xwi15/*/POST/particle-data
// URL: https://kp5m4xwi15.execute-api.us-east-2.amazonaws.com/prod
https://kp5m4xwi15.execute-api.us-east-2.amazonaws.com/prod/particle-data
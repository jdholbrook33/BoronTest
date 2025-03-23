# Pool Flow Monitor - Particle Boron Implementation

## Overview
This project uses a Particle Boron device to monitor pool water flow, calculate gallons used, and send data to AWS DynamoDB for analysis and visualization.

## Hardware
- Particle Boron (LTE cellular IoT device)
- Flow meter sensor connected to D2 pin (1140 pulses per gallon)
- Status LED on D7 pin

## Key Features
- Real-time flow monitoring and calculation
- Automatic fill event detection with 20-second timeout
- LED indicator that flashes during active flow
- Cellular connectivity via Particle Cloud
- Data storage in AWS DynamoDB
- Reset counters between fill events for discrete measurements

## Device Code
The Boron monitors pulses from a flow sensor, calculates gallons used, and publishes data to the Particle cloud when a fill event completes.

Key functionality:
- Interrupt-driven pulse counting
- Rate-based flow detection (>5 pulses per second)
- Automatic detection of fill event completion
- LED indicator flashes during active flow
- Resets counters after publishing

## Data Pipeline
1. Boron device detects fill events
2. Device publishes JSON payload to Particle cloud
3. Particle webhook forwards data to AWS API Gateway
4. API Gateway triggers Lambda function
5. Lambda writes data to DynamoDB

## AWS Resources
- **API Gateway**: Endpoint for Particle webhook
  - URL: https://kp5m4xwi15.execute-api.us-east-2.amazonaws.com/prod/particle-data
- **Lambda Function**: Processes data and writes to DynamoDB
- **DynamoDB Table**: 
  - Table Name: PoolFlowData
  - Partition Key: deviceId (String)
  - Sort Key: timestamp (Number)

## JSON Payload Format
```json
{
  "device_id": "pool_1",
  "timestamp": 1742263411,
  "gallons_used": 6.00,
  "signal_strength": 37
}
```

## Particle Integration Setup
1. In Particle Console, go to Integrations → New Integration → Webhook
2. Configure webhook:
   - Event Name: "flow_data"
   - URL: https://kp5m4xwi15.execute-api.us-east-2.amazonaws.com/prod/particle-data
   - Request Type: POST
   - Request Format: JSON

## Lambda Function
The Lambda function processes the Particle webhook payload and stores it in DynamoDB:

```python
import json
import boto3
from decimal import Decimal

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('PoolFlowData')

def lambda_handler(event, context):
    print(f"Received event: {json.dumps(event)}")
    
    try:
        # Parse the JSON string from the 'data' field
        device_data = json.loads(event['data'])
        print(f"Device data: {json.dumps(device_data)}")
        
        # Prepare DynamoDB item
        item = {
            'deviceId': device_data['device_id'],
            'timestamp': int(device_data['timestamp']),
            'gallonsUsed': Decimal(str(device_data['gallons_used'])),
            'signalStrength': int(device_data['signal_strength'])
        }
        
        # Write to DynamoDB
        table.put_item(Item=item)
        print('Data saved to DynamoDB successfully')
        
        return {
            'statusCode': 200,
            'body': json.dumps({'success': True})
        }
    except Exception as e:
        print(f"Error processing data: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }
```

## Notes
- The first 100 Particle devices can use the platform for free
- Data usage is minimal (~18KB/month per device)
- Consider B-SoM ($46) for production devices
- Complete pipeline validated with real-world testing showing accurate measurements
- Future enhancement: Add external serial display for field diagnostics
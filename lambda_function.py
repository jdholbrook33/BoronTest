import json
import boto3
from decimal import Decimal

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('PoolFlowData')

def lambda_handler(event, context):
    print('Received event:', json.dumps(event))
    
    try:
        # Parse the incoming body from Particle webhook
        body = json.loads(event['body'])
        print('Parsed body:', json.dumps(body))
        
        # Extract the device data
        device_data = json.loads(body['data'])
        print('Device data:', json.dumps(device_data))
        
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
            'headers': {
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'success': True,
                'message': 'Data saved successfully'
            })
        }
    except Exception as e:
        print('Error processing data:', str(e))
        return {
            'statusCode': 500,
            'headers': {
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'success': False,
                'message': 'Error processing data',
                'error': str(e)
            })
        }
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 800 400">
  <!-- Background -->
  <rect width="800" height="400" fill="#f8f9fa" rx="10" ry="10"/>
  
  <!-- Boron Device -->
  <rect x="50" y="150" width="120" height="80" fill="#91d5ff" stroke="#1890ff" stroke-width="2" rx="5" ry="5"/>
  <text x="110" y="190" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">Particle Boron</text>
  <text x="110" y="210" font-family="Arial" font-size="12" text-anchor="middle" fill="#000">Flow Sensor</text>
  
  <!-- Particle Cloud -->
  <rect x="240" y="130" width="120" height="120" fill="#d9f7be" stroke="#52c41a" stroke-width="2" rx="5" ry="5"/>
  <text x="300" y="180" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">Particle</text>
  <text x="300" y="200" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">Cloud</text>
  
  <!-- API Gateway -->
  <rect x="430" y="150" width="120" height="80" fill="#ffd6e7" stroke="#eb2f96" stroke-width="2" rx="5" ry="5"/>
  <text x="490" y="190" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">AWS API</text>
  <text x="490" y="210" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">Gateway</text>
  
  <!-- Lambda Function -->
  <rect x="430" y="290" width="120" height="80" fill="#d3adf7" stroke="#722ed1" stroke-width="2" rx="5" ry="5"/>
  <text x="490" y="330" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">AWS Lambda</text>
  <text x="490" y="350" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">Function</text>
  
  <!-- DynamoDB -->
  <rect x="620" y="150" width="120" height="80" fill="#ffe7ba" stroke="#fa8c16" stroke-width="2" rx="5" ry="5"/>
  <text x="680" y="190" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">AWS</text>
  <text x="680" y="210" font-family="Arial" font-size="16" text-anchor="middle" fill="#000">DynamoDB</text>
  
  <!-- Flow arrows -->
  <!-- Boron to Particle Cloud -->
  <path d="M170 190 H240" stroke="#000" stroke-width="2" fill="none" marker-end="url(#arrowhead)"/>
  <text x="205" y="180" font-family="Arial" font-size="12" text-anchor="middle" fill="#000">Publish Event</text>
  
  <!-- Particle Cloud to API Gateway -->
  <path d="M360 190 H430" stroke="#000" stroke-width="2" fill="none" marker-end="url(#arrowhead)"/>
  <text x="395" y="180" font-family="Arial" font-size="12" text-anchor="middle" fill="#000">Webhook</text>
  
  <!-- API Gateway to Lambda -->
  <path d="M490 230 V290" stroke="#000" stroke-width="2" fill="none" marker-end="url(#arrowhead)"/>
  <text x="510" y="260" font-family="Arial" font-size="12" text-anchor="middle" fill="#000">Trigger</text>
  
  <!-- Lambda to DynamoDB -->
  <path d="M550 330 H590 V230 H620" stroke="#000" stroke-width="2" fill="none" marker-end="url(#arrowhead)"/>
  <text x="600" y="310" font-family="Arial" font-size="12" text-anchor="middle" fill="#000">Store Data</text>
  
  <!-- JSON Payloads -->
  <rect x="100" y="50" width="240" height="40" fill="#fff" stroke="#d9d9d9" stroke-width="1" rx="2" ry="2"/>
  <text x="220" y="75" font-family="Courier New" font-size="10" text-anchor="middle" fill="#000">{"device_id":"pool_1","gallons_used":25.5,...}</text>
  
  <!-- Legend -->
  <rect x="550" y="40" width="200" height="80" fill="#fff" stroke="#d9d9d9" stroke-width="1" rx="5" ry="5"/>
  <text x="650" y="60" font-family="Arial" font-size="14" text-anchor="middle" font-weight="bold" fill="#000">Data Flow</text>
  <text x="580" y="80" font-family="Arial" font-size="12" text-anchor="start" fill="#000">• Device measures flow</text>
  <text x="580" y="100" font-family="Arial" font-size="12" text-anchor="start" fill="#000">• JSON data transmitted</text>
  <text x="580" y="120" font-family="Arial" font-size="12" text-anchor="start" fill="#000">• AWS stores for analysis</text>
  
  <!-- Arrow marker definition -->
  <defs>
    <marker id="arrowhead" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
      <polygon points="0 0, 10 3.5, 0 7" fill="#000"/>
    </marker>
  </defs>
</svg>
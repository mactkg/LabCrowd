var commander = require('commander')
var request = require('request')

commander
  .version('0.0.1')
  .option('-k, --key [key]', 'Set API key')
  .option('-s, --state [state]', 'Set State')
  .parse(process.argv)

console.log(commander)

var key = commander.key
if (key === '') {
  console.error('api key is required')
}

request({
  url: 'http://wedata.net/items/77781',
  method: 'PUT',
  form: {
    'api_key': key,
    'data[crowd]': commander.state
  }
}).on('response', function (response) {
  console.log(response.statusCode)
  console.log(response.headers)
  console.log(response.body)
})

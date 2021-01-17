'use strict';

const HELPER_BASE = process.env.HELPER_BASE || '../../helpers/';
const Response = require(HELPER_BASE + 'response');
const BinResponse = require(HELPER_BASE + 'binresponse');

const { URL, URLSearchParams } = require('url');
const fetch = require('node-fetch');
const Headers = fetch.Headers;

const puppeteer = require('puppeteer');

exports.handler = async (event, context, callback) => {
	if( event.path == '/screenshot' ){
		var url = event.queryStringParameters.url;
		var wait = 0;
		var width = 640;
		var height = 480;
		var scale = 1.0;
		var type = event.queryStringParameters.type || 'png'; // png or jpeg
		if( event.queryStringParameters.width )
			width = parseInt(event.queryStringParameters.width);
		if( event.queryStringParameters.height )
			height = parseInt(event.queryStringParameters.height);
		if( event.queryStringParameters.scale )
			scale = parseFloat(event.queryStringParameters.scale);
		if( event.queryStringParameters.wait )
			wait = parseInt(event.queryStringParameters.wait);
		console.log(width, height, scale, url);

		var browser = await puppeteer.launch();
		var page = await browser.newPage();
		await page.setViewport({
			width: width,
			height: height,
			deviceScaleFactor: scale,
		});
		await page.goto(url, { waitUntil: "load" });
		if( wait > 0 )
			await page.waitForTimeout(wait);
		if( event.queryStringParameters.waitfor ){
			try{
				await page.waitForFunction("vue.render.loaded");
			}catch(error){
				console.log(error);
			}
		}
		var buffer = await page.screenshot({ type: type });
		browser.close();

		return new BinResponse('image/' + type, buffer);
	}else
	if( event.path == '/screenshot-weather'){
		var location = parseInt(event.queryStringParameters.location);
		var weather = await do_get_weather(location);

		return new Response({ weather });
	}
};

/* location: 13:東京、14:神奈川 */
function do_get_weather(location){
	return fetch('https://www.drk7.jp/weather/json/' + location + '.js', {
			method : 'GET'
	})
	.then((response) => {
			return response.text();
	})
	.then(text =>{
			text = text.trim();
			if( text.startsWith('drk7jpweather.callback(') )
					text = text.slice(23, -2);
			return JSON.parse(text);
	});
}

/* location: 13:東京、14:神奈川 */
async function do_get_covid19(location){
	var covid_prefs = await do_get("https://covid19-japan-web-api.now.sh/api/v1/prefectures", {});
	var covid_pref = covid_prefs.find(item => item.id == location );
	var covid_total = await do_get("https://covid19-japan-web-api.now.sh/api/v1/total", {});

	return { total: covid_total, pref: covid_pref };
}

function do_get(url, qs) {
  var params = new URLSearchParams(qs);
  var url2 = new URL(url);
  url2.search = params;

  return fetch(url2.toString(), {
      method: 'GET',
    })
    .then((response) => {
      if (!response.ok)
        throw 'status is not 200';
      return response.json();
    });
}
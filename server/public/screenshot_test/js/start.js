'use strict';

//var vConsole = new VConsole();

const base_url = "【Node.jsサーバのURL】";

var vue_options = {
    el: "#top",
    data: {
        progress_title: '', // for progress-dialog

        message: "initialize",
        render: {},
        weather: null,
        date: "",
    },
    computed: {
    },
    methods: {
        test: function(){
            console.log(vue.message);

        }
    },
    created: async function(){
    },
    mounted: async function(){
        proc_load();

        this.date = new Date().toLocaleDateString("ja-JP", { year: "numeric", month: "2-digit", day: "2-digit" });
        var response = await do_get(base_url + '/screenshot-weather', { location: 14 } );
        this.weather = response.weather.pref.area["東部"].info.find(item => item.date == this.date );
        console.log(this.weather);

        this.render_loaded();
    }
};
vue_add_methods(vue_options, methods_bootstrap);
vue_add_components(vue_options, components_bootstrap);
var vue = new Vue( vue_options );

async function wait_async(msec){
    return new Promise(resolve => setTimeout(resolve, msec));
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

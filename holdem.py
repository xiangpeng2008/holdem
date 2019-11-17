import dash
import dash_core_components as dcc
import dash_html_components as html
from dash.dependencies import Input, Output, State
import json
import flask

import pandas as pd
import numpy as np
import re
import socket

HOST = '127.0.0.1'    # The remote host
PORT = 20002          # The same port as used by the server
serverCpp=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverCpp.connect((HOST, PORT))

app = dash.Dash(
    __name__,
    external_stylesheets=['https://codepen.io/chriddyp/pen/bWLwgP.css'],
    meta_tags=[ {"name": "viewport", "content": "width=device-width, initial-scale=1"} ],
)
fontSize=20
login_form = html.Div([
    html.Form([
        dcc.Input(placeholder='username', name='username', type='text'),
        dcc.Input(placeholder='password', name='password', type='password'),
        html.Button('Login', type='submit')
    ], action='/login', method='post')
])

url_bar_and_content_div = html.Div([
    dcc.Location(id='url', refresh=False),
    html.Div(id='page-content')
])

holdem_form = html.Div( [
    html.Div([
        dcc.Dropdown(
            id='input_option',
            options=[
                {'label': 'new game', 'value': 'newGame`'},
                {'label': 'show people on this table', 'value': 'people_on_table`'},#玩家们
                {'label': 'join room *', 'value': 'join`'},#加入
                {'label': 'create room of * people',   'value': 'create_room'    },
                ],
            value='',
            style={'fontSize': fontSize},
            className='fullScreen',
            ),
        html.Br(),
        dcc.Input(id='input_cmd', type='text', placeholder='Enter your text here.',value='', debounce=True, className='input_cmd', style={'fontSize': fontSize},),
        html.Button(id='submit_button', n_clicks_timestamp=0, n_clicks=0, children='Submit', className='button', style={'fontSize': fontSize}),
        html.Br(),
        html.Br(),
        html.Button(id='submit_cards' , n_clicks=0, children='Cards', className='smallButton', style={'fontSize': fontSize}),
        html.Button(id='submit_pass',   n_clicks_timestamp=0, n_clicks=0, children='Pass',  className='smallButton', style={'fontSize': fontSize}),
        html.Button(id='submit_fold'  , n_clicks_timestamp=0, n_clicks=0, children='Fold',  className='smallButton', style={'fontSize': fontSize}),
        ]),
    html.Div(id='cache_history', style={'display': 'none'}, children="[\"Interaction history\"]"),
    html.Div(id='cache_cards_click', style={'display': 'none'}, children='0'),
    html.Div(id='history'),
    ])

@app.server.route('/login', methods=['POST'])
def route_login():
    data = flask.request.form
    username = data.get('username')
    password = data.get('password')

    if not username or not password=="pass123":
        flask.abort(401)

    # actual implementation should verify the password.
    # Recommended to only keep a hash in database and use something like
    # bcrypt to encrypt the password and check the hashed results.

    # Return a redirect with
    rep = flask.redirect('/holdem')

    # Here we just store the given username in a cookie.
    # Actual session cookies should be signed or use a JWT token.
    rep.set_cookie('custom_auth_session', username)
    return rep

@app.server.route('/guide')
def route_guide():
    return flask.send_from_directory('','guide.html')

@app.server.route('/guide_en')
def route_guide():
    return flask.send_from_directory('','guide_en.html')

# create a logout route
@app.server.route('/logout', methods=['POST'])
def route_logout():
    # Redirect back to the index and remove the session cookie.
    rep = flask.redirect(_app_route)
    rep.set_cookie('custom_auth_session', '', expires=0)
    return rep

def serve_layout():
    if flask.has_request_context():
        return url_bar_and_content_div
    return html.Div([
        url_bar_and_content_div,
        holdem_form,
        ])

app.layout = serve_layout
cards52 = list("🂱🂲🂳🂴🂵🂶🂷🂸🂹🂺🂻🂽🂾🃁🃂🃃🃄🃅🃆🃇🃈🃉🃊🃋🃍🃎🂡🂢🂣🂤🂥🂦🂧🂨🂩🂪🂫🂭🂮🃑🃒🃓🃔🃕🃖🃗🃘🃙🃚🃛🃝🃞")
cards52Span = [ html.Span(cards52[i], style={'color': ('red' if i<26 else 'black'), 'fontSize': 96}) for i in range(52)]

# Index callbacks
@app.callback(Output('page-content', 'children'),
        [Input('url', 'pathname')])
def display_page(pathname):
    if pathname == "/holdem":
        return holdem_form
    elif pathname == "/login":
        return login_form

def getQuery(command, existe_value, isPoker = False):
    session_cookie = flask.request.cookies.get('custom_auth_session')
    if not session_cookie:
        return json.dumps(['not logged in, please go to click [ivoryhuo.com:7778/login](http://ivoryhuo.com:7778/login) to login']),'',''
    command=session_cookie+','+command
    print(command)
    serverCpp.sendall(command.encode('utf-8'))
    qres = serverCpp.recv(1024)
    print("qres is ")
    print(qres)
    if isinstance(qres, pd.core.frame.DataFrame):
        str_df = qres.select_dtypes([np.object])
        str_df = str_df.stack().str.decode('utf-8').unstack()
        for col in str_df:
            qres[col] = str_df[col]
        qres=qres.to_string(index=False)
    elif isinstance(qres, list):
        qres='\n'.join([i.decode('UTF-8') for i in qres])
    else:
        qres=qres.decode('UTF-8')
    if isPoker:
        return qres
    res=json.loads(existe_value)
    return json.dumps((["*"+command+"*","```bash\n"+qres+"\n```"]+res))

@app.callback(
        [ Output('cache_history', 'children'), Output('input_option', 'value'),Output('input_cmd', 'value') ], 
        [ Input('submit_button','n_clicks_timestamp'), Input('submit_pass','n_clicks_timestamp'), Input('submit_fold','n_clicks_timestamp'),],
        [
            State('input_option', 'value'),
            State('input_cmd', 'value'),
            State('cache_history', 'children')
            ]
        )
def update_output_div(tSubmit, tPass, tFold, input_opt, input_cmd, existe_value):
    tClicks = [tSubmit, tPass, tFold]
    lastClicked = tClicks.index(max(tClicks))
    if (tClicks[lastClicked] == 0) or ((not input_opt) and (lastClicked == 0)):
        return [existe_value,'','']
    query = ''
    print(tClicks)
    print(lastClicked)
    if lastClicked == 0:
        query = getQuery(input_opt+( ','+ input_cmd.replace(" ", "") if input_cmd else ''), existe_value)
    elif lastClicked == 1:
        query = getQuery('pass`', existe_value)
    elif lastClicked == 2:
        query = getQuery('fold`', existe_value)
    return query,'',''

@app.callback(
        [Output('history', 'children'), Output('cache_cards_click','children')],
        [Input('cache_history','children'), Input('submit_cards','n_clicks')],
        [State('cache_cards_click', 'children')],
        )
def update_output_div(existe_value, nCard2, prevNCard2):
    prevNCard2 = json.loads(prevNCard2)
    history_res = "";
    if nCard2 > prevNCard2:
        cardsIndex = getQuery('cards`', existe_value, True)
        alphabet = "a b c d e f g"
        print(cardsIndex)
        res = [html.Div([cards52Span[int(i)] for i in j.split(",")]) for j in cardsIndex.split(";")]
        history_res = html.Div(res)
    else:
        history_res = dcc.Markdown('\n'.join(json.loads(existe_value)))
    return (history_res,json.dumps(nCard2))

server = app.server

if __name__ == '__main__':
    app.run_server(port=7778, debug=True, host='0.0.0.0')

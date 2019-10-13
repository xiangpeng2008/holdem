import dash
import dash_core_components as dcc
import dash_html_components as html
from dash.dependencies import Input, Output, State
import json
import flask

from qpython import qconnection
import pandas as pd
import numpy as np

q = qconnection.QConnection(host = 'localhost', port = 7778, pandas = True)
q.open()

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
        html.Button(id='newGame',  n_clicks=0, children='newGame',  className='button', style={'fontSize': fontSize}),
        html.Button(id='newCards', n_clicks=0, children='newCards', className='button', style={'fontSize': fontSize}),
        html.Button(id='myCards',  n_clicks=0, children='myCards',  className='button', style={'fontSize': fontSize}),
        ,]),
    html.Div(id='cache_history', style={'display': 'none'}, children="[\"Interaction history\"]"),
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

# Index callbacks
@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname == "/holdem":
        return holdem_form
    elif pathname == "/login":
        return login_form

@app.callback(
        [ Output('cache_history', 'children'), Output('input_option', 'value'),Output('input_cmd', 'value'), ], 
        [Input('submit_button','n_clicks')],
        [
            State('input_option', 'value'),
            State('input_cmd', 'value'),
            State('cache_history', 'children')]
        )
def update_output_div(click, input_opt, input_cmd, existe_value):
    session_cookie = flask.request.cookies.get('custom_auth_session')
    if not session_cookie:
        return json.dumps(['not logged in, please go to click [ivoryhuo.com/login](http://ivoryhuo.com/login) to login']),'',''
        # return ['not logged in, please go to click [127.0.0.1:8050/login](http://127.0.0.1:8050/login) to login'],'',''
    print(session_cookie)
    if not input_opt:
        return existe_value,'',''
    command='python["'+session_cookie+'";"'+input_opt+('[\\"'+input_cmd.replace(" ", "")+'\\"]' if input_cmd else '')+'"]'
    print('command: '+command)
    qres=q.sendSync(command)
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
    res=json.loads(existe_value)
    return [json.dumps((["*"+input_opt+" "+input_cmd+"*","```bash\n"+qres+"\n```"]+res)),'','']

@app.callback(
        Output('history', 'children'), 
        [Input('cache_history','children')],
        )
def update_output_div(existe_value):
    res = '\n'.join(json.loads(existe_value))
    print(res)
    return dcc.Markdown(res)

server = app.server

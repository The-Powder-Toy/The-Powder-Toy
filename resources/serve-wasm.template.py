from OpenSSL import crypto
import http.server
import json
import os
import re
import ssl
import sys
import time

(
	script,
	build_root,
) = sys.argv

os.chdir(build_root)

HTTP_HOST  = '127.0.0.1'
HTTP_PORT  = 8000
HTTP_INDEX = 'serve-wasm.index.html'
KEY_FILE   = 'serve-wasm.key.pem'
CERT_FILE  = 'serve-wasm.cert.crt'
CERT_CN    = 'serve-wasm.py self-signed certificate for @APP_EXE@.js'
CERT_TTL   = 30 * 24 * 60 * 60

print(f'generating index for @APP_EXE@.js')
with open(HTTP_INDEX, 'wt') as f:
	f.write(f'''
<!DOCTYPE html>
<html lang="en">
	<body>
		<p>Remember, this is being served locally, so it will not be able to connect to <a href="@SERVER@">@SERVER@</a>. To enable this, serve the following files there:</p>
		<ul>
			<li>@APP_EXE@.js</li>
			<li>@APP_EXE@.wasm</li>
			<li>@APP_EXE@.wasm.map for debugging purposes</li>
		</ul>
		<p id="status" style="display: none;">Loading...</p>
		<canvas style="display: none;" class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex=-1></canvas>
		<script type='text/javascript'>
			(() => {{
				var promise;

				window.create_@APP_EXE_JSSAFE@_loader = () => {{
					if (promise === undefined) {{
						promise = new Promise((resolve, reject) => {{
							const script = document.createElement('script');
							script.onload = () => {{
								resolve(window.create_@APP_EXE_JSSAFE@);
							}};
							document.head.appendChild(script);
							script.src = '@APP_EXE@.js';
						}});
					}}
					return promise;
				}};
			}})();
			(() => {{
				var canvas = document.getElementById('canvas');
				var status = document.getElementById('status');
				window.mark_presentable = function() {{
					canvas.style.display = 'initial';
				}};
				window.onerror = (event) => {{
					status.innerText = 'Exception thrown, see JavaScript console';
					status.style.display = 'initial';
				}};
				create_@APP_EXE_JSSAFE@_loader().then(create_@APP_EXE_JSSAFE@ => create_@APP_EXE_JSSAFE@({{
					canvas: (() => {{
						canvas.addEventListener('webglcontextlost', e => {{
							alert('WebGL context lost. You will need to reload the page.'); e.preventDefault();
						}}, false);
						return canvas;
					}})(),
					print: console.log,
					printErr: console.log,
				}}));
			}})();
		</script>
	</body>
</html>
''')

def remove_if_too_old(path):
	if os.path.isfile(path):
		diff = time.time() - os.path.getmtime(path)
		if diff > CERT_TTL / 2:
			os.remove(path)

remove_if_too_old(CERT_FILE)
remove_if_too_old(KEY_FILE)

if not (os.path.isfile(CERT_FILE) and os.path.isfile(KEY_FILE)):
	print('generating keypair')
	key = crypto.PKey()
	key.generate_key(crypto.TYPE_RSA, 2048)
	cert = crypto.X509()
	cert.get_subject().CN = CERT_CN
	cert.set_serial_number(int(time.time()))
	cert.gmtime_adj_notBefore(0)
	cert.gmtime_adj_notAfter(CERT_TTL)
	cert.set_issuer(cert.get_subject())
	cert.set_pubkey(key)
	cert.sign(key, 'sha256')
	with open(CERT_FILE, 'wt') as f:
		f.write(crypto.dump_certificate(crypto.FILETYPE_PEM, cert).decode('utf-8'))
	with open(KEY_FILE, 'wt') as f:
		f.write(crypto.dump_privatekey(crypto.FILETYPE_PEM, key).decode('utf-8'))

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
	index_pages = (HTTP_INDEX, )

	def end_headers(self):
		print(self.index_pages)
		self.send_my_headers()
		http.server.SimpleHTTPRequestHandler.end_headers(self)

	def send_my_headers(self):
		self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
		self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')

server_address = (HTTP_HOST, HTTP_PORT)
httpd = http.server.HTTPServer(server_address, MyHTTPRequestHandler)
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain(certfile=CERT_FILE, keyfile=KEY_FILE)
httpd.socket = ssl_context.wrap_socket(httpd.socket, server_side=True)
print(f'serving at https://{HTTP_HOST}:{HTTP_PORT}, Ctrl+C to exit')
httpd.serve_forever()

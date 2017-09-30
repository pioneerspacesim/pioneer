#!/bin/env python3

import re
import os
import json
import mimetypes
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from systems import GetSystemsSet, UnknownSystem

GET_HANDLERS = []


def OnGet(regex):
  def f(g):
    GET_HANDLERS.append((re.compile(regex), g))
    return g

  return f


@OnGet(r'/$')
def Index(h):
  ServeStatic(h, 'index.html')


@OnGet(r'/json/systems/contents/$')
def GetContents(h):
  h.send_response(200)
  h.send_header('Content-type', 'application/json')
  h.end_headers()
  h.wfile.write(json.dumps(GetSystemsSet().GetContents()).encode('utf-8'))


@OnGet(r'/json/systems/get/$')
def GetSystem(h):
  params = parse_qs(urlparse(h.path).query)
  try:
    x = GetSystemsSet().GetSystem(params['file'][0], params['system'][0])
  except UnknownSystem:
    h.send_response(404)
    h.send_header("Content-type", "text/plain")
    h.end_headers()
    h.wfile.write(b"404 Not found.")
    return

  h.send_response(200)
  h.send_header('Content-type', 'application/json')
  h.end_headers()
  h.wfile.write(json.dumps(x).encode('utf-8'))


@OnGet(r'/static/(.*)')
def ServeStatic(h, file):
  base_dir = os.path.join(
      os.path.dirname(os.path.realpath(__file__)), 'static')
  file_dir = os.path.abspath(os.path.join(base_dir, file))
  if not file_dir.startswith(base_dir):
    h.send_response(403)
    h.send_header("Content-type", "text/plain")
    h.end_headers()
    h.wfile.write(b"403 Fishy request!")
    return

  try:
    with open(file_dir, 'rb') as f:
      h.send_response(200)
      h.send_header("Content-type", mimetypes.guess_type(file_dir, False)[0])
      h.end_headers()
      h.wfile.write(f.read())
  except:
    h.send_response(404)
    h.send_header("Content-type", "text/plain")
    h.end_headers()
    h.wfile.write(b"404 File not found.")


class Handler(BaseHTTPRequestHandler):
  def do_GET(self):
    url = urlparse(self.path)

    for r, f in GET_HANDLERS:
      m = r.match(url.path)
      if m:
        return f(self, *m.groups())

    self.send_response(404)
    self.send_header("Content-type", "text/plain")
    self.end_headers()
    self.wfile.write(b"404 Not found.")


def run():
  h = HTTPServer(('127.0.0.1', 9914), Handler)
  print("Open http://localhost:9914/ in browser.")
  h.serve_forever()


run()
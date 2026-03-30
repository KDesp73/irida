FROM python:3.12-slim-bookworm

RUN apt-get update \
    && apt-get install -y --no-install-recommends stockfish \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY service/requirements.txt ./service/requirements.txt
RUN pip install --no-cache-dir -r service/requirements.txt

COPY service/ ./service/
COPY service/config.docker.yml ./service/config.yml

ENV PYTHONUNBUFFERED=1
EXPOSE 8765

CMD ["sh", "-c", "exec python -m service --serve --host 0.0.0.0 --port ${PORT:-8765}"]

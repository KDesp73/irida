# chess-engine

## Build from source

```bash
$ git clone https://github.com/KDesp73/chess-engine
$ cd chess-engine
$ make all
```

## Testing

```bash
$ make test
```

> [!NOTE]
> Data-driven testing handled 
> by [IncludeOnly/test.h](https://github.com/IncludeOnly/test.h)
> See [test/](./test)


## Configuration

> [!NOTE]
> Lua is being used to handle 
> the engine's configuration 
> and tuning

- `config.lua`: UCI config
- `tuning.lua`: Tuning


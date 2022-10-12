profiles:
	mkdir -p .profiles
	memray run --trace-python-allocators -o .profiles/memray_logrecord.py.bin -f --native memray_logrecord.py
	memray run --trace-python-allocators -o .profiles/memray_format_exception.py.bin -f --native memray_format_exception.py
	memray run --trace-python-allocators -o .profiles/memray_logger.py.bin -f --native memray_logger.py
	memray flamegraph --leaks -f .profiles/memray_logrecord.py.bin
	memray flamegraph --leaks -f .profiles/memray_format_exception.py.bin
	memray flamegraph --leaks -f .profiles/memray_logger.py.bin
	python -m http.server --directory .profiles 8000

clean:
	rm -rf .profiles

snapshot:
	cp -R .profiles .profiles-snapshot
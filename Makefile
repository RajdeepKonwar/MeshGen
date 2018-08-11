all:
	$(MAKE) -C ./GeoGen
	$(MAKE) -C ./EurekaGen

.PHONY: clean

clean:
	$(MAKE) -C ./GeoGen clean
	$(MAKE) -C ./EurekaGen clean

#pragma once

class vmthk
{
public:
	vmthk() = default;

	void setup(size_t TABLE_COUNT, void *inst, void* storage = nullptr)
	{
		memset(this, 0, sizeof(vmthk));

		m_inst = inst;
		m_table_original = *reinterpret_cast<void ***>(inst);
		m_count = TABLE_COUNT;

		if (!m_count)
			return;

		m_len = (m_count + 1 + 1) * sizeof(uptr);
		if(storage != nullptr)
		{
			m_table = reinterpret_cast<void**>(storage);
		} else {
			/*
				This has been stripped because not needed ATM since we use CAVES,
				and it causes an until yet unnoticed compilation error:
					r5/src/inc/vmthk.h:23:47: error: use of undeclared identifier 'g'
                } else {m_table = reinterpret_cast<void **>(g.ix.mm->alloc(m_len)); }
                                                            ^
				Ideally we put this in a vmthk.cpp or some shit, should we need it again.
			*/
			// m_table = reinterpret_cast<void **>(g.ix.mm->alloc(m_len));
#if LOGGING_IS_ENABLED == 1
			LMsg(128, "vmthk::setup() -> null storage not implemented!");
#endif
			__fastfail(_encoded_const(0xDAC1FD00));
		}

		if (!m_table)
			return;

		memset(m_table, 0, m_len);

		m_table[0] = m_table_original[-1];
		memcpy(&m_table[1], m_table_original, m_len - (sizeof(uptr) * 2));
	}

	const auto &count() const { return m_count; }

	auto table() { return m_table; }
	auto enable() { *reinterpret_cast<uptr *>(m_inst) = BASE_OF(&m_table[1]); }
	auto disable() { *reinterpret_cast<void ***>(m_inst) = m_table_original; }

	auto place(size_t idx2, void *handler)  -> void *
	{
		auto idx = idx2 + 1;
		if (idx > m_count)
			return nullptr;

		auto orig = m_table[idx];
		m_table[idx] = handler;

		return orig;
	}

	template <typename R>
	auto original(size_t idx) -> R
	{
		return (R)m_table_original[idx];
	}

private:
private:
	void *m_inst;
	void **m_table;
	void **m_table_original;
	size_t m_count;
	size_t m_len;
};
